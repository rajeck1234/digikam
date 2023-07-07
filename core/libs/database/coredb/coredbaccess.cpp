/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Core database access wrapper.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbaccess.h"

// Qt includes

#include <QEventLoop>
#include <QMutex>
#include <QMutexLocker>
#include <QSqlDatabase>
#include <QUuid>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "coredb.h"
#include "collectionscannerobserver.h"
#include "iteminfodata.h"
#include "iteminfocache.h"
#include "coredbschemaupdater.h"
#include "collectionmanager.h"
#include "coredbwatch.h"
#include "coredbbackend.h"
#include "dbengineerrorhandler.h"
#include "tagscache.h"
#include "dbengineparameters.h"
#include "dbengineaccess.h"

namespace Digikam
{

class Q_DECL_HIDDEN CoreDbAccessStaticPriv
{
public:

    CoreDbAccessStaticPriv()
        : backend              (nullptr),
          db                   (nullptr),
          databaseWatch        (nullptr),
          // Create a unique identifier for this application (as an application accessing a database
          applicationIdentifier(QUuid::createUuid()),
          initializing         (false)
    {
    };

    ~CoreDbAccessStaticPriv()
    {
    };

public:

    CoreDbBackend*      backend;
    CoreDB*             db;
    CoreDbWatch*        databaseWatch;
    DbEngineParameters  parameters;
    DbEngineLocking     lock;
    QString             lastError;
    QUuid               applicationIdentifier;

    bool                initializing;
};

CoreDbAccessStaticPriv* CoreDbAccess::d = nullptr;

// -----------------------------------------------------------------------------

class Q_DECL_HIDDEN CoreDbAccessMutexLocker

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))

    : public QMutexLocker<QRecursiveMutex>

#else

    : public QMutexLocker

#endif

{
public:

    explicit CoreDbAccessMutexLocker(CoreDbAccessStaticPriv* const dd)
        : QMutexLocker(&dd->lock.mutex),
          d           (dd)
    {
        d->lock.lockCount++;
    }

    ~CoreDbAccessMutexLocker()
    {
        d->lock.lockCount--;
    }

public:

    CoreDbAccessStaticPriv* const d;
};

// -----------------------------------------------------------------------------

CoreDbAccess::CoreDbAccess()
{
    // You will want to call setParameters before constructing CoreDbAccess

    Q_ASSERT(d);

    d->lock.mutex.lock();
    d->lock.lockCount++;

    if (!d->backend->isOpen() && !d->initializing)
    {
        // avoid endless loops (e.g. recursing from CollectionManager)

        d->initializing = true;

        d->backend->open(d->parameters);
        d->databaseWatch->setDatabaseIdentifier(d->db->databaseUuid().toString());
        CollectionManager::instance()->refresh();

        d->initializing = false;
    }
}

CoreDbAccess::~CoreDbAccess()
{
    d->lock.lockCount--;
    d->lock.mutex.unlock();
}

CoreDbAccess::CoreDbAccess(bool)
{
    // private constructor, when mutex is locked and
    // backend should not be checked

    d->lock.mutex.lock();
    d->lock.lockCount++;
}

CoreDB* CoreDbAccess::db() const
{
    return d->db;
}

CoreDbBackend* CoreDbAccess::backend() const
{
    return d->backend;
}

CoreDbWatch* CoreDbAccess::databaseWatch()
{
    if (d)
    {
        return d->databaseWatch;
    }

    return nullptr;
}

void CoreDbAccess::initDbEngineErrorHandler(DbEngineErrorHandler* const errorhandler)
{
    if (!d || !d->backend)
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: please set parameters before setting a database error handler";
        return;
    }

    d->backend->setDbEngineErrorHandler(errorhandler);
}

DbEngineParameters CoreDbAccess::parameters()
{
    if (d)
    {
        return d->parameters;
    }

    return DbEngineParameters();
}

void CoreDbAccess::setParameters(const DbEngineParameters& parameters)
{
    setParameters(parameters, DatabaseSlave);

    if (d->databaseWatch)
    {
        d->databaseWatch->doAnyProcessing();
    }
}

void CoreDbAccess::setParameters(const DbEngineParameters& parameters, ApplicationStatus status)
{
    if (!d)
    {
        d = new CoreDbAccessStaticPriv();
    }

    CoreDbAccessMutexLocker lock(d);

    if (d->parameters == parameters)
    {
        return;
    }

    if (d->backend && d->backend->isOpen())
    {
        d->backend->close();
    }

    // Kill the old database error handler

    if (d->backend)
    {
        d->backend->setDbEngineErrorHandler(nullptr);
    }

    d->parameters = parameters;

    if (!d->databaseWatch)
    {
        d->databaseWatch = new CoreDbWatch();
        d->databaseWatch->setApplicationIdentifier(d->applicationIdentifier.toString());

        if (status == MainApplication)
        {
            d->databaseWatch->initializeRemote(CoreDbWatch::DatabaseMaster);
        }
        else
        {
            d->databaseWatch->initializeRemote(CoreDbWatch::DatabaseSlave);
        }
    }

    ItemInfoStatic::create();

    if (!d->backend || !d->backend->isCompatible(parameters))
    {
        delete d->db;
        delete d->backend;
        d->backend = new CoreDbBackend(&d->lock);
        d->backend->setCoreDbWatch(d->databaseWatch);
        d->db      = new CoreDB(d->backend);
        TagsCache::instance()->initialize();
    }

    d->databaseWatch->sendDatabaseChanged();
    ItemInfoStatic::cache()->invalidate();
    TagsCache::instance()->invalidate();
    d->databaseWatch->setDatabaseIdentifier(QString());
    CollectionManager::instance()->clearLocations();
}

bool CoreDbAccess::checkReadyForUse(InitializationObserver* const observer)
{
    if (!DbEngineAccess::checkReadyForUse(d->lastError))
    {
        return false;
    }

    if (!DbEngineConfig::checkReadyForUse())
    {
        d->lastError = DbEngineConfig::errorMessage();

        // Make sure the application does not continue to run

        if (observer)
        {
            observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    // Create an object with private shortcut constructor

    CoreDbAccess access(false);

    if (!d->backend)
    {
        qCWarning(DIGIKAM_COREDB_LOG) << "Core database: no database backend available in checkReadyForUse. "
                                         "Did you call setParameters before?";
        return false;
    }

    if (d->backend->isReady())
    {
        return true;
    }

    // TODO: Implement a method to wait until the database is open

    if (!d->backend->isOpen())
    {
        if (!d->backend->open(d->parameters))
        {
            access.setLastError(i18n("Error opening database backend.\n%1",
                                d->backend->lastError()));
            return false;
        }
    }

    // Avoid endless loops (if called methods create new CoreDbAccess objects)

    d->initializing = true;

    // Update schema

    CoreDbSchemaUpdater updater(access.db(), access.backend(), access.parameters());
    updater.setCoreDbAccess(&access);
    updater.setObserver(observer);

    // Check or set WAL mode for SQLite database from DbEngineParameters

    d->backend->checkOrSetWALMode();

    if (!d->backend->initSchema(&updater))
    {
        qCWarning(DIGIKAM_COREDB_LOG) << "Core database: cannot process schema initialization";

        access.setLastError(updater.getLastErrorMessage());
        d->initializing = false;
        return false;
    }

    // Set identifier again

    d->databaseWatch->setDatabaseIdentifier(d->db->databaseUuid().toString());

    // Initialize CollectionManager

    CollectionManager::instance()->refresh();

    d->initializing = false;

    return d->backend->isReady();
}

QString CoreDbAccess::lastError()
{
    return d->lastError;
}

void CoreDbAccess::setLastError(const QString& error)
{
    d->lastError = error;
}

void CoreDbAccess::cleanUpDatabase()
{
    if (d)
    {
        CoreDbAccessMutexLocker locker(d);

        if (d->backend)
        {
            d->backend->close();
            delete d->db;
            delete d->backend;
            delete d->databaseWatch;
        }
    }

    ItemInfoStatic::destroy();
    delete d;
    d = nullptr;
}

// ----------------------------------------------------------------------

CoreDbAccessUnlock::CoreDbAccessUnlock()
{
    // acquire lock

    CoreDbAccess::d->lock.mutex.lock();

    // store lock count

    count = CoreDbAccess::d->lock.lockCount;

    // set lock count to 0

    CoreDbAccess::d->lock.lockCount = 0;

    // unlock

    for (int i = 0 ; i < count ; ++i)
    {
        CoreDbAccess::d->lock.mutex.unlock();
    }

    // drop lock acquired in first line. Mutex is now free.

    CoreDbAccess::d->lock.mutex.unlock();
}

CoreDbAccessUnlock::CoreDbAccessUnlock(CoreDbAccess* const)
{
    // With the passed pointer, we have assured that the mutex is acquired
    // Store lock count

    count = CoreDbAccess::d->lock.lockCount;

    // set lock count to 0

    CoreDbAccess::d->lock.lockCount = 0;

    // unlock

    for (int i = 0 ; i < count ; ++i)
    {
        CoreDbAccess::d->lock.mutex.unlock();
    }

    // Mutex is now free
}

CoreDbAccessUnlock::~CoreDbAccessUnlock()
{
    // lock as often as it was locked before

    for (int i = 0 ; i < count ; ++i)
    {
        CoreDbAccess::d->lock.mutex.lock();
    }

    // update lock count

    CoreDbAccess::d->lock.lockCount += count;
}

} // namespace Digikam
