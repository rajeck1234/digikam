/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-16
 * Description : Core database Schema updater
 *
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "coredbschemaupdater.h"

// Qt includes

#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "drawdecoder.h"
#include "digikam_debug.h"
#include "coredbbackend.h"
#include "coredbtransaction.h"
#include "coredbchecker.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "collectionscanner.h"
#include "itemquerybuilder.h"
#include "collectionscannerobserver.h"
#include "digikam_config.h"
#include "digikam_globals.h"

namespace Digikam
{

int CoreDbSchemaUpdater::schemaVersion()
{
    return 16;
}

int CoreDbSchemaUpdater::filterSettingsVersion()
{
    return 17;
}

int CoreDbSchemaUpdater::uniqueHashVersion()
{
    return 2;
}

bool CoreDbSchemaUpdater::isUniqueHashUpToDate()
{
    return (CoreDbAccess().db()->getUniqueHashVersion() >= uniqueHashVersion());
}

// --------------------------------------------------------------------------------------

class Q_DECL_HIDDEN CoreDbSchemaUpdater::Private
{

public:

    explicit Private()
      : setError(false),
        backend (nullptr),
        albumDB (nullptr),
        dbAccess(nullptr),
        observer(nullptr)
    {
    }

    bool                    setError;

    QVariant                currentVersion;
    QVariant                currentRequiredVersion;

    CoreDbBackend*          backend;
    CoreDB*                 albumDB;
    DbEngineParameters      parameters;

    // legacy
    CoreDbAccess*           dbAccess;

    QString                 lastErrorMessage;
    InitializationObserver* observer;
};

CoreDbSchemaUpdater::CoreDbSchemaUpdater(CoreDB* const albumDB,
                                         CoreDbBackend* const backend,
                                         const DbEngineParameters& parameters)
    : d(new Private)
{
    d->backend    = backend;
    d->albumDB    = albumDB;
    d->parameters = parameters;
}

CoreDbSchemaUpdater::~CoreDbSchemaUpdater()
{
    delete d;
}

void CoreDbSchemaUpdater::setCoreDbAccess(CoreDbAccess* const dbAccess)
{
    d->dbAccess = dbAccess;
}

const QString CoreDbSchemaUpdater::getLastErrorMessage()
{
    return d->lastErrorMessage;
}

bool CoreDbSchemaUpdater::update()
{
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: running schema update";
    bool success = startUpdates();

    // cancelled?

    if (d->observer && !d->observer->continueQuery())
    {
        return false;
    }

    // even on failure, try to set current version - it may have incremented

    setVersionSettings();

    if (!success)
    {
        return false;
    }

    updateFilterSettings();

    if (d->observer)
    {
        d->observer->finishedSchemaUpdate(InitializationObserver::UpdateSuccess);
    }

    return success;
}

void CoreDbSchemaUpdater::setVersionSettings()
{
    if (d->currentVersion.isValid())
    {
        d->albumDB->setSetting(QLatin1String("DBVersion"),
                               QString::number(d->currentVersion.toInt()));
    }

    if (d->currentRequiredVersion.isValid())
    {
        d->albumDB->setSetting(QLatin1String("DBVersionRequired"),
                               QString::number(d->currentRequiredVersion.toInt()));
    }
}

static QVariant safeToVariant(const QString& s)
{
    if (s.isEmpty())
    {
        return QVariant();
    }
    else
    {
        return s.toInt();
    }
}

void CoreDbSchemaUpdater::readVersionSettings()
{
    d->currentVersion         = safeToVariant(d->albumDB->getSetting(QLatin1String("DBVersion")));
    d->currentRequiredVersion = safeToVariant(d->albumDB->getSetting(QLatin1String("DBVersionRequired")));
}

void CoreDbSchemaUpdater::setObserver(InitializationObserver* const observer)
{
    d->observer = observer;
}

bool CoreDbSchemaUpdater::startUpdates()
{
    // Do we have sufficient privileges

    QStringList insufficientRights;
    CoreDbPrivilegesChecker checker(d->parameters);

    if (!checker.checkPrivileges(insufficientRights))
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: insufficient rights on database.";

        QString errorMsg = i18n("You have insufficient privileges on the database.\n"
                                "Following privileges are not assigned to you:\n %1\n"
                                "Check your privileges on the database and restart digiKam.",
                                insufficientRights.join(QLatin1String(",\n")));
        d->lastErrorMessage = errorMsg;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

         return false;
    }

    // First step: do we have an empty database?

    QStringList tables = d->backend->tables();

    if (tables.contains(QLatin1String("Albums"), Qt::CaseInsensitive))
    {
        // Find out schema version of db file

        readVersionSettings();
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: have a structure version " << d->currentVersion.toInt();

        // We absolutely require the DBVersion setting

        if (!d->currentVersion.isValid())
        {
            // Something is damaged. Give up.

            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: version not available! Giving up schema upgrading.";

            QString errorMsg = i18n("The database is not valid: "
                                    "the \"DBVersion\" setting does not exist. "
                                    "The current database schema version cannot be verified. "
                                    "Try to start with an empty database. ");
            d->lastErrorMessage=errorMsg;

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        // current version describes the current state of the schema in the db,
        // schemaVersion is the version required by the program.

        if (d->currentVersion.toInt() > schemaVersion())
        {
            // trying to open a database with a more advanced than this CoreDbSchemaUpdater supports

            if (d->currentRequiredVersion.isValid() && d->currentRequiredVersion.toInt() <= schemaVersion())
            {
                // version required may be less than current version

                return true;
            }
            else
            {
                QString errorMsg = i18n("The database has been used with a more recent version of digiKam "
                                        "and has been updated to a database schema which cannot be used with this version. "
                                        "(This means this digiKam version is too old, or the database format is too recent.) "
                                        "Please use the more recent version of digiKam that you used before. ");
                d->lastErrorMessage=errorMsg;

                if (d->observer)
                {
                    d->observer->error(errorMsg);
                    d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
                }

                return false;
            }
        }
        else
        {
            return makeUpdates();
        }
    }
    else
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: no database file available";

        // Legacy handling?

        // first test if there are older files that need to be upgraded.
        // This applies to "digikam.db" for 0.7 and "digikam3.db" for 0.8 and 0.9,
        // all only SQLite databases.

        // Version numbers used in this source file are a bit confused for the historic versions.
        // Version 1 is 0.6 (no db), Version 2 is 0.7 (SQLite 2),
        // Version 3 is 0.8-0.9,
        // Version 3 wrote the setting "DBVersion", "1",
        // Version 4 is 0.10, the digikam3.db file copied to digikam4.db, no schema changes.
        // Version 4 writes "4", and from now on version x writes "x".
        // Version 5 includes the schema changes from 0.9 to 0.10
        // Version 6 brought new tables for history and ImageTagProperties, with version 2.0
        // Version 7 brought the VideoMetadata table with 3.0

        if (d->parameters.isSQLite())
        {
            QFileInfo currentDBFile(d->parameters.databaseNameCore);
            QFileInfo digikam3DB(currentDBFile.dir(), QLatin1String("digikam3.db"));

            if (digikam3DB.exists())
            {
                if (!copyV3toV4(digikam3DB.filePath(), currentDBFile.filePath()))
                {
                    return false;
                }

                // d->currentVersion is now 4;

                return makeUpdates();
            }
        }

        // No legacy handling: start with a fresh db

        if (!createDatabase() || !createFilterSettings())
        {
            QString errorMsg    = i18n("Failed to create tables in database.\n ") + d->backend->lastError();
            d->lastErrorMessage = errorMsg;

            if (d->observer)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }

            return false;
        }

        return true;
    }
}

bool CoreDbSchemaUpdater::beginWrapSchemaUpdateStep()
{
    if (!d->backend->beginTransaction())
    {
        QFileInfo currentDBFile(d->parameters.databaseNameCore);
        QString errorMsg = i18n("Failed to open a database transaction on your database file \"%1\". "
                                "This is unusual. Please check that you can access the file and no "
                                "other process has currently locked the file. "
                                "If the problem persists you can get help from the digikam developers mailing list (see www.digikam.org/support). "
                                "As well, please have a look at what digiKam prints on the console. ",
                                QDir::toNativeSeparators(currentDBFile.filePath()));
        d->observer->error(errorMsg);
        d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);

        return false;
    }

    return true;
}

bool CoreDbSchemaUpdater::endWrapSchemaUpdateStep(bool stepOperationSuccess, const QString& errorMsg)
{
    if (!stepOperationSuccess)
    {
        d->backend->rollbackTransaction();

        if (d->observer)
        {
            // error or cancelled?

            if      (!d->observer->continueQuery())
            {
                qCDebug(DIGIKAM_COREDB_LOG) << "Core database: schema update cancelled by user";
            }
            else if (!d->setError)
            {
                d->observer->error(errorMsg);
                d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
            }
        }

        return false;
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: success updating to version " << d->currentVersion.toInt();
    d->backend->commitTransaction();

    return true;
}

bool CoreDbSchemaUpdater::makeUpdates()
{
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: makeUpdates " << d->currentVersion.toInt() << " to " << schemaVersion();

    if (d->currentVersion.toInt() < schemaVersion())
    {
        if (d->currentVersion.toInt() < 5)
        {
            if (!beginWrapSchemaUpdateStep())
            {
                return false;
            }

            // v4 was always SQLite

            QFileInfo currentDBFile(d->parameters.databaseNameCore);
            QString errorMsg = i18n("The schema updating process from version 4 to 6 failed, "
                                    "caused by an error that we did not expect. "
                                    "You can try to discard your old database and start with an empty one. "
                                    "(In this case, please move the database files "
                                    "\"%1\" and \"%2\" from the directory \"%3\"). "
                                    "More probably you will want to report this error to the digikam developers mailing list (see www.digikam.org/support). "
                                    "As well, please have a look at what digiKam prints on the console. ",
                                    QLatin1String("digikam3.db"),
                                    QLatin1String("digikam4.db"),
                                    QDir::toNativeSeparators(currentDBFile.dir().path()));

            if (!endWrapSchemaUpdateStep(updateV4toV7(), errorMsg))
            {
                return false;
            }

            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: success updating v4 to v6";

            // Still set these even in >= 1.4 because 0.10 - 1.3 may want to apply the updates if not set

            setLegacySettingEntries();
        }

        // Incremental updates, starting from version 5

        for (int v = d->currentVersion.toInt() ; v < schemaVersion() ; ++v)
        {
            int targetVersion = v + 1;

            if (!beginWrapSchemaUpdateStep())
            {
                return false;
            }

            QString errorMsg;

            if (d->parameters.internalServer)
            {
                errorMsg = i18n("Failed to update the database schema from version %1 to version %2.\n"
                                "The cause could be a missing upgrade of the database to the current "
                                "server version. Now start digiKam again to perform a required "
                                "upgrade of the database.",
                                d->currentVersion.toInt(),
                                targetVersion);
            }
            else
            {
                errorMsg = i18n("Failed to update the database schema from version %1 to version %2.\n"
                                "Please read the error messages printed on the console and "
                                "report this error as a bug at bugs.kde.org.",
                                d->currentVersion.toInt(),
                                targetVersion);
            }

            if (!endWrapSchemaUpdateStep(updateToVersion(targetVersion), errorMsg))
            {
                return false;
            }

            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: success updating to version " << d->currentVersion.toInt();
        }

        // add future updates here
    }

    return true;
}

void CoreDbSchemaUpdater::defaultFilterSettings(QStringList& defaultItemFilter, QStringList& defaultVideoFilter,
                                                QStringList& defaultAudioFilter)
{
    // NOTE for updating:
    // When changing anything here, just increment filterSettingsVersion() so that the changes take effect

    // https://en.wikipedia.org/wiki/Image_file_formats

    defaultItemFilter << QLatin1String("jpg")  << QLatin1String("jpeg") << QLatin1String("jpe")   // JPEG
                      << QLatin1String("mpo")
                      << QLatin1String("jp2")  << QLatin1String("j2k")  << QLatin1String("jpx")   // JPEG-2000
                      << QLatin1String("jpc")  << QLatin1String("pgx")
                      << QLatin1String("tif")  << QLatin1String("tiff")                           // Tagged Image Format
                      << QLatin1String("png")                                                     // Portable Network Graphic
                      << QLatin1String("eps")                                                     // Encapsulated Postscript
                      << QLatin1String("fit")  << QLatin1String("fts")  << QLatin1String("fits")  // Flexible Image Transport System (https://fr.wikipedia.org/wiki/Flexible_Image_Transport_System)
                      << QLatin1String("gif")                                                     // Graphics Interchange Format
                      << QLatin1String("xbm")  << QLatin1String("xpm")                            // X Bitmap and Pixmap
                      << QLatin1String("ppm")  << QLatin1String("pbm")  << QLatin1String("pgm")   // Portable Pixmap
                      << QLatin1String("pnm")
                      << QLatin1String("pic")                                                     // PICtor Image Format
                      << QLatin1String("cur ") << QLatin1String("ico")  << QLatin1String("icns")  // Windows/Apple Cursor and Icon
                      << QLatin1String("pgf")                                                     // Portable Graphics Format
                      << QLatin1String("bmp")                                                     // Windows Bitmap
                      << QLatin1String("pcx")                                                     // Picture Exchange
                      << QLatin1String("tga")                                                     // Truevision Graphics Adapter
                      << QLatin1String("sgi")  << QLatin1String("rgb")  << QLatin1String("rgba")  // Silicon Graphics Image
                      << QLatin1String("bw")
                      << QLatin1String("heic") << QLatin1String("heif") << QLatin1String("hif")   // High Efficiency Image File Format
                      << QLatin1String("jxl")                                                     // JPEG-XL
                      << QLatin1String("avif")                                                    // AV1 Image File Format
                      << QLatin1String("wbmp")                                                    // Wireless Application Protocol Bitmap Format
                      << QLatin1String("webp")                                                    // Web Photo
    ;

    // Raster graphics editor containers: https://en.wikipedia.org/wiki/Raster_graphics_editor

    defaultItemFilter << QLatin1String("xcf") << QLatin1String("psd") << QLatin1String("psb")
                      << QLatin1String("kra") << QLatin1String("ora") << QLatin1String("wmf");

    // Raw images: https://en.wikipedia.org/wiki/Raw_image_format

    defaultItemFilter << DRawDecoder::rawFilesList();

    // Video files: https://en.wikipedia.org/wiki/Video_file_format

    defaultVideoFilter << QLatin1String("mpeg") << QLatin1String("mpg") << QLatin1String("mpe")  // MPEG
                       << QLatin1String("mts")  << QLatin1String("vob")
                       << QLatin1String("avi")  << QLatin1String("divx")                         // RIFF
                       << QLatin1String("wmv")  << QLatin1String("asf")                          // ASF
                       << QLatin1String("mp4")  << QLatin1String("3gp") << QLatin1String("mov")  // QuickTime
                       << QLatin1String("3g2")  << QLatin1String("m4v") << QLatin1String("m2v")
                       << QLatin1String("mkv")  << QLatin1String("webm")                         // Matroska
                       << QLatin1String("mng")                                                   // Animated PNG image
                       << QLatin1String("m2ts");                                                 // Blu-ray

    // Audio files: https://en.wikipedia.org/wiki/Audio_file_format

    defaultAudioFilter << QLatin1String("ogg") << QLatin1String("oga") << QLatin1String("flac")  // Linux audio
                       << QLatin1String("wv")  << QLatin1String("ape") << QLatin1String("mpc")
                       << QLatin1String("au")
                       << QLatin1String("m4b") << QLatin1String("aax") << QLatin1String("aa")    // Book audio
                       << QLatin1String("mp3") << QLatin1String("aac")                           // MPEG based audio
                       << QLatin1String("m4a") << QLatin1String("m4p") << QLatin1String("caf")   // Apple audio
                       << QLatin1String("aiff")
                       << QLatin1String("wma") << QLatin1String("wav");                          // Windows audio
}

void CoreDbSchemaUpdater::defaultIgnoreDirectoryFilterSettings(QStringList& defaultIgnoreDirectoryFilter)
{
    // NOTE: when update this section,
    // just increment filterSettingsVersion() so that the changes take effect

    defaultIgnoreDirectoryFilter << QLatin1String("@eaDir");
}

bool CoreDbSchemaUpdater::createFilterSettings()
{
    QStringList defaultItemFilter, defaultVideoFilter, defaultAudioFilter, defaultIgnoreDirectoryFilter;
    defaultFilterSettings(defaultItemFilter, defaultVideoFilter, defaultAudioFilter);
    defaultIgnoreDirectoryFilterSettings(defaultIgnoreDirectoryFilter);

    d->albumDB->setFilterSettings(defaultItemFilter, defaultVideoFilter, defaultAudioFilter);
    d->albumDB->setIgnoreDirectoryFilterSettings(defaultIgnoreDirectoryFilter);
    d->albumDB->setSetting(QLatin1String("FilterSettingsVersion"),      QString::number(filterSettingsVersion()));
    d->albumDB->setSetting(QLatin1String("DcrawFilterSettingsVersion"), QString::number(DRawDecoder::rawFilesVersion()));

    return true;
}

bool CoreDbSchemaUpdater::updateFilterSettings()
{
    QString filterVersion      = d->albumDB->getSetting(QLatin1String("FilterSettingsVersion"));
    QString dcrawFilterVersion = d->albumDB->getSetting(QLatin1String("DcrawFilterSettingsVersion"));

    if (filterVersion.toInt() < filterSettingsVersion() ||
        dcrawFilterVersion.toInt() < DRawDecoder::rawFilesVersion())
    {
        createFilterSettings();
    }

    return true;
}

bool CoreDbSchemaUpdater::createDatabase()
{
    if (createTables() && createIndices() && createTriggers())
    {
        setLegacySettingEntries();

        d->currentVersion = schemaVersion();

        // if we start with the V2 hash, version 6 is required
        d->albumDB->setUniqueHashVersion(uniqueHashVersion());
        d->currentRequiredVersion = schemaVersion();
/*
        // digiKam for database version 5 can work with version 6, though not using the new features
        d->currentRequiredVersion = 5;
*/
        return true;
    }
    else
    {
        return false;
    }
}

bool CoreDbSchemaUpdater::createTables()
{
    return d->backend->execDBAction(d->backend->getDBAction(QLatin1String("CreateDB")));
}

bool CoreDbSchemaUpdater::createIndices()
{
    // TODO: see which more indices are needed
    // create indices

    return d->backend->execDBAction(d->backend->getDBAction(QLatin1String("CreateIndices")));
}

bool CoreDbSchemaUpdater::createTriggers()
{
    return d->backend->execDBAction(d->backend->getDBAction(QLatin1String("CreateTriggers")));
}

bool CoreDbSchemaUpdater::updateUniqueHash()
{
    if (isUniqueHashUpToDate())
    {
        return true;
    }

    readVersionSettings();

    {
        CoreDbTransaction transaction;

        CoreDbAccess().db()->setUniqueHashVersion(uniqueHashVersion());

        CollectionScanner scanner;
        scanner.setNeedFileCount(true);
        scanner.setUpdateHashHint();

        if (d->observer)
        {
            d->observer->connectCollectionScanner(&scanner);
            scanner.setObserver(d->observer);
        }

        scanner.completeScan();

        // earlier digiKam does not know about the hash

        if (d->currentRequiredVersion.toInt() < 6)
        {
            d->currentRequiredVersion = 6;
            setVersionSettings();
        }
    }
    return true;
}

bool CoreDbSchemaUpdater::performUpdateToVersion(const QString& actionName, int newVersion, int newRequiredVersion)
{
    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->moreSchemaUpdateSteps(1);
    }

    DbEngineAction updateAction = d->backend->getDBAction(actionName);

    if (updateAction.name.isNull())
    {
        QString errorMsg = i18n("The database update action cannot be found. Please ensure that "
                                "the dbconfig.xml file of the current version of digiKam is installed "
                                "at the correct place. ");
    }

    if (!d->backend->execDBAction(updateAction))
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: schema update to V" << newVersion << "failed!";

        // resort to default error message, set above

        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Updated schema to version %1.", newVersion));
    }

    d->currentVersion = newVersion;

    // digiKam for database version 5 can work with version 6, though not using the new features
    // Note: We do not upgrade the uniqueHash

    d->currentRequiredVersion = newRequiredVersion;

    return true;
}

bool CoreDbSchemaUpdater::updateToVersion(int targetVersion)
{
    if (d->currentVersion != targetVersion-1)
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: updateToVersion performs only incremental updates. Called to update from"
                                    << d->currentVersion << "to" << targetVersion << ", aborting.";
        return false;
    }

    switch (targetVersion)
    {
        case 6:
        {
            // digiKam for database version 5 can work with version 6,
            // though not using the new features.
            // Note: We do not upgrade the uniqueHash.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV5ToV6"), 6, 5);
        }

        case 7:
        {
            // digiKam for database version 5 and 6 can work with version 7,
            // though not using the support for video files.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV6ToV7"), 7, 5);

            // NOTE: If you add a new update step, please check the
            // d->currentVersion at the bottom of updateV4toV7.
            // If the update already comes with createTables,
            // createTriggers, we don't need the extra update here.
        }

        case 8:
        {
            // digiKam for database version 7 can work with version 8,
            // now using COLLATE utf8_general_ci for MySQL.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV7ToV9"), 8, 5);
        }

        case 9:
        {
            // digiKam for database version 8 can work with version 9,
            // now using COLLATE utf8_general_ci for MySQL.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV7ToV9"), 9, 5);
        }

        case 10:
        {
            // digiKam for database version 9 can work with version 10,
            // remove ImageHaarMatrix table and add manualOrder column.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV9ToV10"), 10, 5);
        }

        case 11:
        {
            // digiKam for database version 10 can work with version 11,
            // add TagsTree table for MySQL.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV10ToV11"), 11, 5);
        }

        case 12:
        {
            // digiKam for database version 11 can work with version 12,
            // fix TagsTree triggers for MySQL.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV11ToV12"), 12, 5);
        }

        case 13:
        {
            // digiKam for database version 12 can work with version 13,
            // add index to the TagsTree table for MySQL.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV12ToV13"), 13, 5);
        }

        case 14:
        {
            // digiKam for database version 13 can work with version 14,
            // add modificationDate column to the Albums table.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV13ToV14"), 14, 5);
        }

        case 15:
        {
            // digiKam for database version 14 can work with version 15,
            // now using COLLATE utf8_bin for Albums:relativePath and Images:name in MySQL.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV14ToV15"), 15, 5);
        }

        case 16:
        {
            // digiKam for database version 15 can work with version 16,
            // add caseSensitivity column to the AlbumRoots table.

            return performUpdateToVersion(QLatin1String("UpdateSchemaFromV15ToV16"), 16, 5);
        }

        default:
        {
            qCDebug(DIGIKAM_COREDB_LOG) << "Core database: unsupported update to version" << targetVersion;
            return false;
        }
    }
}

bool CoreDbSchemaUpdater::copyV3toV4(const QString& digikam3DBPath, const QString& currentDBPath)
{
    if (d->observer)
    {
        d->observer->moreSchemaUpdateSteps(2);
    }

    d->backend->close();

    // We cannot use KIO here because KIO only works from the main thread

    QFile oldFile(digikam3DBPath);
    QFile newFile(currentDBPath);

    // QFile won't override. Remove the empty db file created when a non-existent file is opened

    newFile.remove();

    if (!oldFile.copy(currentDBPath))
    {
        QString errorMsg = i18n("Failed to copy the old database file (\"%1\") "
                                "to its new location (\"%2\"). "
                                "Error message: \"%3\". "
                                "Please make sure that the file can be copied, "
                                "or delete it.",
                                digikam3DBPath,
                                currentDBPath,
                                oldFile.errorString());
        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Copied database file"));
    }

    if (!d->backend->open(d->parameters))
    {
        QString errorMsg = i18n("The old database file (\"%1\") has been copied "
                                "to the new location (\"%2\") but it cannot be opened. "
                                "Please delete both files and try again, "
                                "starting with an empty database. ",
                                digikam3DBPath,
                                currentDBPath);

        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Opened new database file"));
    }

    d->currentVersion = 4;

    return true;
}

static QStringList cleanUserFilterString(const QString& filterString)
{
    // splits by either ; or space, removes "*.", trims

    QStringList filterList;

    QString wildcard(QLatin1String("*."));
    QChar dot(QLatin1Char('.'));
    Q_UNUSED(dot);

    QChar sep(QLatin1Char(';'));
    int i = filterString.indexOf( sep );

    if ( i == -1 && filterString.indexOf(QLatin1Char(' ')) != -1 )
    {
        sep = QChar(QLatin1Char(' '));
    }

    QStringList sepList = filterString.split(sep, QT_SKIP_EMPTY_PARTS);

    Q_FOREACH (const QString& f, sepList)
    {
        if (f.startsWith(wildcard))
        {
            filterList << f.mid(2).trimmed().toLower();
        }
        else
        {
            filterList << f.trimmed().toLower();
        }
    }

    return filterList;
}

bool CoreDbSchemaUpdater::updateV4toV7()
{
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database : running updateV4toV7";

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->moreSchemaUpdateSteps(11);
    }

    // This update was introduced from digikam version 0.9 to digikam 0.10
    // We operator on an SQLite3 database under a transaction (which will be rolled back on error)

    // --- Make space for new tables ---

    if (!d->backend->execSql(QString::fromUtf8("ALTER TABLE Albums RENAME TO AlbumsV3;")))
    {
        return false;
    }

    if (!d->backend->execSql(QString::fromUtf8("ALTER TABLE Images RENAME TO ImagesV3;")))
    {
        return false;
    }

    if (!d->backend->execSql(QString::fromUtf8("ALTER TABLE Searches RENAME TO SearchesV3;")))
    {
        return false;
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: moved tables";

    // --- Drop some triggers and indices ---

    // Don't check for errors here. The "IF EXISTS" clauses seem not supported in SQLite

    d->backend->execSql(QString::fromUtf8("DROP TRIGGER delete_album;"));
    d->backend->execSql(QString::fromUtf8("DROP TRIGGER delete_image;"));
    d->backend->execSql(QString::fromUtf8("DROP TRIGGER delete_tag;"));
    d->backend->execSql(QString::fromUtf8("DROP TRIGGER insert_tagstree;"));
    d->backend->execSql(QString::fromUtf8("DROP TRIGGER delete_tagstree;"));
    d->backend->execSql(QString::fromUtf8("DROP TRIGGER move_tagstree;"));
    d->backend->execSql(QString::fromUtf8("DROP INDEX dir_index;"));
    d->backend->execSql(QString::fromUtf8("DROP INDEX tag_index;"));

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Prepared table creation"));
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: dropped triggers";

    // --- Create new tables ---

    if (!createTables() || !createIndices())
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Created tables"));
    }

    // --- Populate AlbumRoots (from config) ---

    KSharedConfigPtr config  = KSharedConfig::openConfig();
    KConfigGroup group       = config->group(QLatin1String("Album Settings"));
    QString albumLibraryPath = group.readEntry(QLatin1String("Album Path"), QString());

    if (albumLibraryPath.isEmpty())
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: Album library path from config file is empty. Aborting update.";

        QString errorMsg    = i18n("No album library path has been found in the configuration file. "
                                   "Giving up the schema updating process. "
                                   "Please try with an empty database, or repair your configuration.");
        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    QUrl albumLibrary(QUrl::fromLocalFile(albumLibraryPath));
    CollectionLocation location = CollectionManager::instance()->addLocation(albumLibrary, albumLibrary.fileName());

    if (location.isNull())
    {
        qCDebug(DIGIKAM_COREDB_LOG) << "Core database: failure to create a collection location. Aborting update.";

        QString errorMsg    = i18n("There was an error associating your albumLibraryPath (\"%1\") "
                                   "with a storage volume of your system. "
                                   "This problem may indicate that there is a problem with your installation. "
                                   "If you are working on Linux, check that HAL is installed and running. "
                                   "In any case, you can seek advice from the digikam developers mailing list (see www.digikam.org/support). "
                                   "The database updating process will now be aborted because we do not want "
                                   "to create a new database based on false assumptions from a broken installation.",
                                   albumLibraryPath);
        d->lastErrorMessage = errorMsg;
        d->setError         = true;

        if (d->observer)
        {
            d->observer->error(errorMsg);
            d->observer->finishedSchemaUpdate(InitializationObserver::UpdateErrorMustAbort);
        }

        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Configured one album root"));
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: inserted album root";

    // --- With the album root, populate albums ---

    if (!d->backend->execSql(QString::fromUtf8(
                                "REPLACE INTO Albums "
                                " (id, albumRoot, relativePath, date, caption, collection, icon) "
                                "SELECT id, ?, url, date, caption, collection, icon "
                                " FROM AlbumsV3;"
                            ),
                            location.id())
       )
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported albums"));
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: populated albums";

    // --- Add images ---

    if (!d->backend->execSql(QString::fromUtf8(
                                "REPLACE INTO Images "
                                " (id, album, name, status, category, modificationDate, fileSize, uniqueHash) "
                                "SELECT id, dirid, name, ?, ?, NULL, NULL, NULL"
                                " FROM ImagesV3;"
                            ),
                            DatabaseItem::Visible, DatabaseItem::UndefinedCategory)
       )
    {
        return false;
    }

    if (!d->dbAccess->backend()->execSql(QString::fromUtf8(
                                          "REPLACE INTO ImageInformation (imageId) SELECT id FROM Images;"))
       )
    {
        return false;
    }

    // remove orphan images that would not be removed by CollectionScanner

    d->backend->execSql(QString::fromUtf8("DELETE FROM Images WHERE album NOT IN (SELECT id FROM Albums);"));

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported images information"));
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: populated Images";

    // --- Port searches ---

    if (!d->backend->execSql(QString::fromUtf8(
                                "REPLACE INTO Searches "
                                " (id, type, name, query) "
                                "SELECT id, ?, name, url"
                                " FROM SearchesV3;"),
                            DatabaseSearch::LegacyUrlSearch)
       )
    {
        return false;
    }

    SearchInfo::List sList = d->albumDB->scanSearches();

    for (SearchInfo::List::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        QUrl url((*it).query);

        ItemQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);
        QString name  = (*it).name;

        if (name == i18n("Last Search"))
        {
            name = i18n("Last Search (0.9)");
        }

        if      (QUrlQuery(url).queryItemValue(QLatin1String("type")) == QLatin1String("datesearch"))
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, name, query);
        }
        else if (QUrlQuery(url).queryItemValue(QLatin1String("1.key")) == QLatin1String("keyword"))
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::KeywordSearch, name, query);
        }
        else
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, name, query);
        }
    }

    // --- Create triggers ---

    if (!createTriggers())
    {
        return false;
    }

    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: created triggers";

    // --- Populate name filters ---

    createFilterSettings();

    // --- Set user settings from config ---

    QStringList defaultItemFilter, defaultVideoFilter, defaultAudioFilter;
    defaultFilterSettings(defaultItemFilter, defaultVideoFilter, defaultAudioFilter);

    auto listFileFilter    = cleanUserFilterString(group.readEntry(QLatin1String("File Filter"),       QString()));
    auto listRawFileFilter = cleanUserFilterString(group.readEntry(QLatin1String("Raw File Filter"),   QString()));
    auto listMovieFilter   = cleanUserFilterString(group.readEntry(QLatin1String("Movie File Filter"), QString()));
    auto listAudioFilter   = cleanUserFilterString(group.readEntry(QLatin1String("Audio File Filter"), QString()));

    QSet<QString> configItemFilter(listFileFilter.begin(), listFileFilter.end());
    configItemFilter  += QSet<QString>(listRawFileFilter.begin(), listRawFileFilter.end());
    QSet<QString> configVideoFilter(listMovieFilter.begin(), listMovieFilter.end());
    QSet<QString> configAudioFilter(listAudioFilter.begin(), listAudioFilter.end());

    // remove those that are included in the default filter

    configItemFilter.subtract(QSet<QString>(defaultItemFilter.begin(), defaultItemFilter.end()));
    configVideoFilter.subtract(QSet<QString>(defaultVideoFilter.begin(), defaultVideoFilter.end()));
    configAudioFilter.subtract(QSet<QString>(defaultAudioFilter.begin(), defaultAudioFilter.end()));

    d->albumDB->setUserFilterSettings(configItemFilter.values(), configVideoFilter.values(), configAudioFilter.values());
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: set initial filter settings with user settings" << configItemFilter;

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Initialized and imported file suffix filter"));
    }

    // --- do a full scan ---

    CollectionScanner scanner;

    if (d->observer)
    {
        d->observer->connectCollectionScanner(&scanner);
        scanner.setObserver(d->observer);
    }

    scanner.completeScan();

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Did the initial full scan"));
    }

    // --- Port date, comment and rating (_after_ the scan) ---

    // Port ImagesV3.date -> ImageInformation.creationDate

    if (!d->backend->execSql(QString::fromUtf8(
                                "UPDATE ImageInformation SET "
                                " creationDate=(SELECT datetime FROM ImagesV3 WHERE ImagesV3.id=ImageInformation.imageid) "
                                "WHERE imageid IN (SELECT id FROM ImagesV3);")
                            )
       )
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported creation dates"));
    }

    // Port ImagesV3.comment to ItemComments

    // An author of NULL will inhibt the UNIQUE restriction to take effect (but #189080). Work around.

    d->backend->execSql(QString::fromUtf8(
                           "DELETE FROM ImageComments WHERE "
                           "type=? AND language=? AND author IS NULL "
                           "AND imageid IN ( SELECT id FROM ImagesV3 ); "),
                       (int)DatabaseComment::Comment, QLatin1String("x-default"));

    if (!d->backend->execSql(QString::fromUtf8(
                                "REPLACE INTO ImageComments "
                                " (imageid, type, language, comment) "
                                "SELECT id, ?, ?, caption FROM ImagesV3;"
                            ),
                            (int)DatabaseComment::Comment, QLatin1String("x-default"))
       )
    {
        return false;
    }

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported comments"));
    }

    // Port rating storage in ImageProperties to ItemInformation

    if (!d->backend->execSql(QString::fromUtf8(
                                "UPDATE ImageInformation SET "
                                " rating=(SELECT value FROM ImageProperties "
                                "         WHERE ImageInformation.imageid=ImageProperties.imageid AND ImageProperties.property=?) "
                                "WHERE imageid IN (SELECT imageid FROM ImageProperties WHERE property=?);"
                            ),
                            QString::fromUtf8("Rating"), QString::fromUtf8("Rating"))
       )
    {
        return false;
    }

    d->backend->execSql(QString::fromUtf8("DELETE FROM ImageProperties WHERE property=?;"), QString::fromUtf8("Rating"));
    d->backend->execSql(QString::fromUtf8("UPDATE ImageInformation SET rating=0 WHERE rating<0;"));

    if (d->observer)
    {
        if (!d->observer->continueQuery())
        {
            return false;
        }

        d->observer->schemaUpdateProgress(i18n("Imported ratings"));
    }

    // --- Drop old tables ---

    d->backend->execSql(QString::fromUtf8("DROP TABLE ImagesV3;"));
    d->backend->execSql(QString::fromUtf8("DROP TABLE AlbumsV3;"));
    d->backend->execSql(QString::fromUtf8("DROP TABLE SearchesV3;"));

    if (d->observer)
    {
        d->observer->schemaUpdateProgress(i18n("Dropped v3 tables"));
    }

    d->currentRequiredVersion = 5;
    d->currentVersion         = 7;
    qCDebug(DIGIKAM_COREDB_LOG) << "Core database: returning true from updating to 5";

    return true;
}

void CoreDbSchemaUpdater::setLegacySettingEntries()
{
    d->albumDB->setSetting(QLatin1String("preAlpha010Update1"), QLatin1String("true"));
    d->albumDB->setSetting(QLatin1String("preAlpha010Update2"), QLatin1String("true"));
    d->albumDB->setSetting(QLatin1String("preAlpha010Update3"), QLatin1String("true"));
    d->albumDB->setSetting(QLatin1String("beta010Update1"),     QLatin1String("true"));
    d->albumDB->setSetting(QLatin1String("beta010Update2"),     QLatin1String("true"));
}

// ---------- Legacy code ------------

void CoreDbSchemaUpdater::preAlpha010Update1()
{
    QString hasUpdate = d->albumDB->getSetting(QLatin1String("preAlpha010Update1"));

    if (!hasUpdate.isNull())
    {
        return;
    }

    if (!d->backend->execSql(QString::fromUtf8("ALTER TABLE Searches RENAME TO SearchesV3;")))
    {
        return;
    }

    if ( !d->backend->execSql(
             QString::fromUtf8( "CREATE TABLE IF NOT EXISTS Searches  \n"
                                " (id INTEGER PRIMARY KEY, \n"
                                "  type INTEGER, \n"
                                "  name TEXT NOT NULL, \n"
                                "  query TEXT NOT NULL);" ) ))
    {
        return;
    }

    if (!d->backend->execSql(QString::fromUtf8( "REPLACE INTO Searches "
                                                " (id, type, name, query) "
                                                "SELECT id, ?, name, url"
                                                " FROM SearchesV3;"),
                             DatabaseSearch::LegacyUrlSearch)
       )
    {
        return;
    }

    SearchInfo::List sList = d->albumDB->scanSearches();

    for (SearchInfo::List::const_iterator it = sList.constBegin(); it != sList.constEnd(); ++it)
    {
        QUrl url((*it).query);

        ItemQueryBuilder builder;
        QString query = builder.convertFromUrlToXml(url);

        if      (QUrlQuery(url).queryItemValue(QLatin1String("type")) == QLatin1String("datesearch"))
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::TimeLineSearch, (*it).name, query);
        }
        else if (QUrlQuery(url).queryItemValue(QLatin1String("1.key")) == QLatin1String("keyword"))
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::KeywordSearch, (*it).name, query);
        }
        else
        {
            d->albumDB->updateSearch((*it).id, DatabaseSearch::AdvancedSearch, (*it).name, query);
        }
    }

    d->backend->execSql(QString::fromUtf8("DROP TABLE SearchesV3;"));

    d->albumDB->setSetting(QLatin1String("preAlpha010Update1"), QLatin1String("true"));
}

void CoreDbSchemaUpdater::preAlpha010Update2()
{
    QString hasUpdate = d->albumDB->getSetting(QLatin1String("preAlpha010Update2"));

    if (!hasUpdate.isNull())
    {
        return;
    }

    if (!d->backend->execSql(QString::fromUtf8("ALTER TABLE ImagePositions RENAME TO ItemPositionsTemp;")))
    {
        return;
    }

    if (!d->backend->execSql(QString::fromUtf8("ALTER TABLE ImageMetadata RENAME TO ImageMetadataTemp;")))
    {
        return;
    }

    d->backend->execSql(
        QString::fromUtf8("CREATE TABLE ItemPositions\n"
                          " (imageid INTEGER PRIMARY KEY,\n"
                          "  latitude TEXT,\n"
                          "  latitudeNumber REAL,\n"
                          "  longitude TEXT,\n"
                          "  longitudeNumber REAL,\n"
                          "  altitude REAL,\n"
                          "  orientation REAL,\n"
                          "  tilt REAL,\n"
                          "  roll REAL,\n"
                          "  accuracy REAL,\n"
                          "  description TEXT);") );

    d->backend->execSql(QString::fromUtf8("REPLACE INTO ImagePositions "
                                          " (imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                                          "  altitude, orientation, tilt, roll, accuracy, description) "
                                          "SELECT imageid, latitude, latitudeNumber, longitude, longitudeNumber, "
                                          "  altitude, orientation, tilt, roll, 0, description "
                                          " FROM ItemPositionsTemp;"));

    d->backend->execSql(
        QString::fromUtf8("CREATE TABLE ImageMetadata\n"
                          " (imageid INTEGER PRIMARY KEY,\n"
                          "  make TEXT,\n"
                          "  model TEXT,\n"
                          "  lens TEXT,\n"
                          "  aperture REAL,\n"
                          "  focalLength REAL,\n"
                          "  focalLength35 REAL,\n"
                          "  exposureTime REAL,\n"
                          "  exposureProgram INTEGER,\n"
                          "  exposureMode INTEGER,\n"
                          "  sensitivity INTEGER,\n"
                          "  flash INTEGER,\n"
                          "  whiteBalance INTEGER,\n"
                          "  whiteBalanceColorTemperature INTEGER,\n"
                          "  meteringMode INTEGER,\n"
                          "  subjectDistance REAL,\n"
                          "  subjectDistanceCategory INTEGER);") );

    d->backend->execSql( QString::fromUtf8("INSERT INTO ImageMetadata "
                                           " (imageid, make, model, lens, aperture, focalLength, focalLength35, "
                                           "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                           "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) "
                                           "SELECT imageid, make, model, NULL, aperture, focalLength, focalLength35, "
                                           "  exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, "
                                           "  whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory "
                                           "FROM ImageMetadataTemp;"));

    d->backend->execSql(QString::fromUtf8("DROP TABLE ItemPositionsTemp;"));
    d->backend->execSql(QString::fromUtf8("DROP TABLE ImageMetadataTemp;"));

    d->albumDB->setSetting(QLatin1String("preAlpha010Update2"), QLatin1String("true"));
}

void CoreDbSchemaUpdater::preAlpha010Update3()
{
    QString hasUpdate = d->albumDB->getSetting(QLatin1String("preAlpha010Update3"));

    if (!hasUpdate.isNull())
    {
        return;
    }

    d->backend->execSql(QString::fromUtf8("DROP TABLE ItemCopyright;"));
    d->backend->execSql(QString::fromUtf8("CREATE TABLE ItemCopyright\n"
                                          " (imageid INTEGER,\n"
                                          "  property TEXT,\n"
                                          "  value TEXT,\n"
                                          "  extraValue TEXT,\n"
                                          "  UNIQUE(imageid, property, value, extraValue));")
    );

    d->albumDB->setSetting(QLatin1String("preAlpha010Update3"), QLatin1String("true"));
}

void CoreDbSchemaUpdater::beta010Update1()
{
    QString hasUpdate = d->albumDB->getSetting(QLatin1String("beta010Update1"));

    if (!hasUpdate.isNull())
    {
        return;
    }

    // if Image has been deleted

    d->backend->execSql(QString::fromUtf8("DROP TRIGGER delete_image;"));
    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER delete_image DELETE ON Images\n"
                                          "BEGIN\n"
                                          "  DELETE FROM ImageTags\n"
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ImageHaarMatrix\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ItemInformation\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ImageMetadata\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ItemPositions\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ItemComments\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ItemCopyright\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ImageProperties\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  UPDATE Albums SET icon=null \n "
                                          "    WHERE icon=OLD.id;\n"
                                          "  UPDATE Tags SET icon=null \n "
                                          "    WHERE icon=OLD.id;\n"
                                          "END;"));

    d->albumDB->setSetting(QLatin1String("beta010Update1"), QLatin1String("true"));
}

void CoreDbSchemaUpdater::beta010Update2()
{
    QString hasUpdate = d->albumDB->getSetting(QLatin1String("beta010Update2"));

    if (!hasUpdate.isNull())
    {
        return;
    }

    // force rescan and creation of ImageInformation entry for videos and audio

    d->backend->execSql(QString::fromUtf8("DELETE FROM Images WHERE category=2 OR category=3;"));

    d->albumDB->setSetting(QLatin1String("beta010Update2"), QLatin1String("true"));
}

bool CoreDbSchemaUpdater::createTablesV3()
{
    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE Albums\n"
                                                " (id INTEGER PRIMARY KEY,\n"
                                                "  url TEXT NOT NULL UNIQUE,\n"
                                                "  date DATE NOT NULL,\n"
                                                "  caption TEXT,\n"
                                                "  collection TEXT,\n"
                                                "  icon INTEGER);") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE Tags\n"
                                                " (id INTEGER PRIMARY KEY,\n"
                                                "  pid INTEGER,\n"
                                                "  name TEXT NOT NULL,\n"
                                                "  icon INTEGER,\n"
                                                "  iconkde TEXT,\n"
                                                "  UNIQUE (name, pid));") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE TagsTree\n"
                                                " (id INTEGER NOT NULL,\n"
                                                "  pid INTEGER NOT NULL,\n"
                                                "  UNIQUE (id, pid));") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE Images\n"
                                                " (id INTEGER PRIMARY KEY,\n"
                                                "  name TEXT NOT NULL,\n"
                                                "  dirid INTEGER NOT NULL,\n"
                                                "  caption TEXT,\n"
                                                "  datetime DATETIME,\n"
                                                "  UNIQUE (name, dirid));") ))
    {
        return false;
    }


    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE ImageTags\n"
                                                " (imageid INTEGER NOT NULL,\n"
                                                "  tagid INTEGER NOT NULL,\n"
                                                "  UNIQUE (imageid, tagid));") ))
    {
        return false;
    }

    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE ImageProperties\n"
                                                " (imageid  INTEGER NOT NULL,\n"
                                                "  property TEXT    NOT NULL,\n"
                                                "  value    TEXT    NOT NULL,\n"
                                                "  UNIQUE (imageid, property));") ))
    {
        return false;
    }

    if ( !d->backend->execSql( QString::fromUtf8("CREATE TABLE Searches  \n"
                                                  " (id INTEGER PRIMARY KEY, \n"
                                                  "  name TEXT NOT NULL UNIQUE, \n"
                                                  "  url  TEXT NOT NULL);") ) )
    {
        return false;
    }

    if (!d->backend->execSql( QString::fromUtf8("CREATE TABLE Settings         \n"
                                                "(keyword TEXT NOT NULL UNIQUE,\n"
                                                " value TEXT);") ))
    {
        return false;
    }

    // TODO: see which more indices are needed
    // create indices

    d->backend->execSql(QString::fromUtf8("CREATE INDEX dir_index ON Images    (dirid);"));
    d->backend->execSql(QString::fromUtf8("CREATE INDEX tag_index ON ImageTags (tagid);"));

    // create triggers

    // trigger: delete from Images/ImageTags/ImageProperties
    // if Album has been deleted

    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER delete_album DELETE ON Albums\n"
                                          "BEGIN\n"
                                          " DELETE FROM ImageTags\n"
                                          "   WHERE imageid IN (SELECT id FROM Images WHERE dirid=OLD.id);\n"
                                          " DELETE From ImageProperties\n"
                                          "   WHERE imageid IN (SELECT id FROM Images WHERE dirid=OLD.id);\n"
                                          " DELETE FROM Images\n"
                                          "   WHERE dirid = OLD.id;\n"
                                          "END;"));

    // trigger: delete from ImageTags/ImageProperties
    // if Image has been deleted

    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER delete_image DELETE ON Images\n"
                                          "BEGIN\n"
                                          "  DELETE FROM ImageTags\n"
                                          "    WHERE imageid=OLD.id;\n"
                                          "  DELETE From ImageProperties\n "
                                          "    WHERE imageid=OLD.id;\n"
                                          "  UPDATE Albums SET icon=null \n "
                                          "    WHERE icon=OLD.id;\n"
                                          "  UPDATE Tags SET icon=null \n "
                                          "    WHERE icon=OLD.id;\n"
                                          "END;"));

    // trigger: delete from ImageTags if Tag has been deleted

    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER delete_tag DELETE ON Tags\n"
                                          "BEGIN\n"
                                          "  DELETE FROM ImageTags WHERE tagid=OLD.id;\n"
                                          "END;"));

    // trigger: insert into TagsTree if Tag has been added

    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags\n"
                                          "BEGIN\n"
                                          "  INSERT INTO TagsTree\n"
                                          "    SELECT NEW.id, NEW.pid\n"
                                          "    UNION\n"
                                          "    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;\n"
                                          "END;"));

    // trigger: delete from TagsTree if Tag has been deleted

    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER delete_tagstree DELETE ON Tags\n"
                                          "BEGIN\n"
                                          " DELETE FROM Tags\n"
                                          "   WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                                          " DELETE FROM TagsTree\n"
                                          "   WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);\n"
                                          " DELETE FROM TagsTree\n"
                                          "    WHERE id=OLD.id;\n"
                                          "END;"));

    // trigger: delete from TagsTree if Tag has been deleted

    d->backend->execSql(QString::fromUtf8("CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags\n"
                                          "BEGIN\n"
                                          "  DELETE FROM TagsTree\n"
                                          "    WHERE\n"
                                          "      ((id = OLD.id)\n"
                                          "        OR\n"
                                          "        id IN (SELECT id FROM TagsTree WHERE pid=OLD.id))\n"
                                          "      AND\n"
                                          "      pid IN (SELECT pid FROM TagsTree WHERE id=OLD.id);\n"
                                          "  INSERT INTO TagsTree\n"
                                          "     SELECT NEW.id, NEW.pid\n"
                                          "     UNION\n"
                                          "     SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid\n"
                                          "     UNION\n"
                                          "     SELECT id, NEW.pid FROM TagsTree WHERE pid=NEW.id\n"
                                          "     UNION\n"
                                          "     SELECT A.id, B.pid FROM TagsTree A, TagsTree B\n"
                                          "        WHERE\n"
                                          "        A.pid = NEW.id AND B.id = NEW.pid;\n"
                                          "END;"));

    return true;
}

} // namespace Digikam
