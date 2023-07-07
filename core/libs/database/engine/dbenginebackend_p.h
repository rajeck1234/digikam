/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Database engine abstract database backend
 *
 * SPDX-FileCopyrightText: 2007-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_ENGINE_BACKEND_PRIVATE_H
#define DIGIKAM_DB_ENGINE_BACKEND_PRIVATE_H

#include "dbenginebackend.h"

// Qt includes

#include <QHash>
#include <QSqlDatabase>
#include <QThread>
#include <QThreadStorage>
#include <QWaitCondition>

// Local includes

#include "digikam_export.h"
#include "dbengineparameters.h"
#include "dbengineerrorhandler.h"

namespace Digikam
{

class Q_DECL_HIDDEN DbEngineThreadData
{
public:

    explicit DbEngineThreadData();
    ~DbEngineThreadData();

    void closeDatabase();

public:

    QSqlDatabase database;
    int          valid;
    int          transactionCount;
    QSqlError    lastError;
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT BdEngineBackendPrivate : public DbEngineErrorAnswer
{
public:

    explicit BdEngineBackendPrivate(BdEngineBackend* const backend);
    ~BdEngineBackendPrivate()                                              override;

    void init(const QString& connectionName, DbEngineLocking* const locking);

    QString connectionName();

    QSqlDatabase databaseForThread();
    QSqlError    databaseErrorForThread();
    void         setDatabaseErrorForThread(const QSqlError& lastError);

    QSqlDatabase createDatabaseConnection();
    void closeDatabaseForThread();
    bool incrementTransactionCount();
    bool decrementTransactionCount();

    bool isInMainThread()                                            const;
    bool isInUIThread()                                              const;

    bool reconnectOnError()                                          const;
    bool isSQLiteLockError(const DbEngineSqlQuery& query)            const;
    bool isSQLiteLockTransactionError(const QSqlError& lastError)    const;
    bool isConnectionError(const DbEngineSqlQuery& query)            const;
    bool needToConsultUserForError(const DbEngineSqlQuery& query)    const;
    bool needToHandleWithErrorHandler(const DbEngineSqlQuery& query) const;
    void debugOutputFailedQuery(const QSqlQuery& query)              const;
    void debugOutputFailedTransaction(const QSqlError& error)        const;

    bool checkRetrySQLiteLockError(int retries);
    bool checkOperationStatus();
    bool handleWithErrorHandler(const DbEngineSqlQuery* const query);
    void setQueryOperationFlag(BdEngineBackend::QueryOperationStatus status);
    void queryOperationWakeAll(BdEngineBackend::QueryOperationStatus status);

    /// called by DbEngineErrorHandler, implementing DbEngineErrorAnswer
    void connectionErrorContinueQueries()                                  override;
    void connectionErrorAbortQueries()                                     override;

    virtual void transactionFinished();

public:

    QThreadStorage<DbEngineThreadData*>       threadDataStorage;

    /**
     * This compares to DbEngineThreadData's valid.
     * If currentValidity is increased and > valid, the db is marked as invalid
     */
    int                                       currentValidity;

    bool                                      isInTransaction;

    QString                                   backendName;

    DbEngineParameters                        parameters;

    BdEngineBackend::Status                   status;

    DbEngineLocking*                          lock;

    BdEngineBackend::QueryOperationStatus     operationStatus;

    QMutex                                    errorLockMutex;
    QWaitCondition                            errorLockCondVar;
    BdEngineBackend::QueryOperationStatus     errorLockOperationStatus;

    QMutex                                    busyWaitMutex;
    QWaitCondition                            busyWaitCondVar;

    DbEngineErrorHandler*                     errorHandler;

public:

    class Q_DECL_HIDDEN AbstractUnlocker
    {
    public:

        explicit AbstractUnlocker(BdEngineBackendPrivate* const d);
        ~AbstractUnlocker();

        void finishAcquire();

    protected:

        int                           count;
        BdEngineBackendPrivate* const d;
    };

    friend class AbstractUnlocker;

    // ------------------------------------------------------------------

    class Q_DECL_HIDDEN AbstractWaitingUnlocker : public AbstractUnlocker
    {
    public:

        explicit AbstractWaitingUnlocker(BdEngineBackendPrivate* const d,
                                         QMutex* const mutex,
                                         QWaitCondition* const condVar);
        ~AbstractWaitingUnlocker();

        bool wait(unsigned long time = ULONG_MAX);

    protected:

        QMutex*         const mutex;
        QWaitCondition* const condVar;
    };

    // ------------------------------------------------------------------

    class Q_DECL_HIDDEN ErrorLocker : public AbstractWaitingUnlocker
    {
    public:

        explicit ErrorLocker(BdEngineBackendPrivate* const d);
        void wait();
    };

    // ------------------------------------------------------------------

    class Q_DECL_HIDDEN BusyWaiter : public AbstractWaitingUnlocker
    {
    public:

        explicit BusyWaiter(BdEngineBackendPrivate* const d);
    };

public:

    BdEngineBackend* const q;
};

} // namespace Digikam

#endif // DIGIKAM_DB_ENGINE_BACKEND_PRIVATE_H
