/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-18
 * Description : Database Engine storage container for connection parameters.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Marcel Wiesweg  <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Mario Frank     <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbengineparameters.h"

// Qt includes

#include <QDir>
#include <QUrlQuery>
#include <QFile>
#include <QCryptographicHash>
#include <QStandardPaths>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "digikam_config.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "o0simplecrypt.h"      // For password encrypt

namespace
{

static const char* configGroupDatabase                       = "Database Settings";
static const char* configInternalDatabaseServer              = "Internal Database Server";
static const char* configInternalDatabaseServerPath          = "Internal Database Server Path";
static const char* configInternalDatabaseServerMysqlAdminCmd = "Internal Database Server Mysql Admin Command";
static const char* configInternalDatabaseServerMysqlServCmd  = "Internal Database Server Mysql Server Command";
static const char* configInternalDatabaseServerMysqlInitCmd  = "Internal Database Server Mysql Init Command";
static const char* configDatabaseType                        = "Database Type";
static const char* configDatabaseName                        = "Database Name";              ///< For Sqlite the DB file path, for Mysql the DB name
static const char* configDatabaseNameThumbnails              = "Database Name Thumbnails";   ///< For Sqlite the DB file path, for Mysql the DB name
static const char* configDatabaseNameFace                    = "Database Name Face";         ///< For Sqlite the DB file path, for Mysql the DB name
static const char* configDatabaseNameSimilarity              = "Database Name Similarity";   ///< For Sqlite the DB file path, for Mysql the DB name
static const char* configDatabaseHostName                    = "Database Hostname";
static const char* configDatabasePort                        = "Database Port";
static const char* configDatabaseUsername                    = "Database Username";
static const char* configDatabasePassword                    = "Database Password";          ///< For compatbilitity. Use encrypted version instead.
static const char* configDatabaseEncryptedPassword           = "Database Encrypted Password";
static const char* configDatabaseConnectOptions              = "Database Connectoptions";
static const char* configDatabaseWALMode                     = "Database WAL Mode";

/// Legacy for older versions.
static const char* configDatabaseFilePathEntry               = "Database File Path";
static const char* configAlbumPathEntry                      = "Album Path";
/// Sqlite DB file names
static const char* digikam4db                                = "digikam4.db";
static const char* thumbnails_digikamdb                      = "thumbnails-digikam.db";
static const char* face_digikamdb                            = "recognition.db";
static const char* similarity_digikamdb                      = "similarity.db";

}

namespace Digikam
{

QString DbEngineParameters::internalServerPrivatePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) +
                                            QLatin1String("/digikam/");
}

DbEngineParameters::DbEngineParameters()
    : port          (-1),
      walMode       (false),
      internalServer(false)
{
}

DbEngineParameters::DbEngineParameters(const QString& _type,
                                       const QString& _databaseNameCore,
                                       const QString& _connectOptions,
                                       const QString& _hostName,
                                       int            _port,
                                       bool           _walMode,
                                       bool           _internalServer,
                                       const QString& _userName,
                                       const QString& _password,
                                       const QString& _databaseNameThumbnails,
                                       const QString& _databaseNameFace,
                                       const QString& _databaseNameSimilarity,
                                       const QString& _internalServerDBPath,
                                       const QString& _internalServerMysqlAdminCmd,
                                       const QString& _internalServerMysqlServCmd,
                                       const QString& _internalServerMysqlInitCmd)
    : databaseType                  (_type),
      databaseNameCore              (_databaseNameCore),
      connectOptions                (_connectOptions),
      hostName                      (_hostName),
      port                          (_port),
      walMode                       (_walMode),
      internalServer                (_internalServer),
      userName                      (_userName),
      password                      (_password),
      databaseNameThumbnails        (_databaseNameThumbnails),
      databaseNameFace              (_databaseNameFace),
      databaseNameSimilarity        (_databaseNameSimilarity),
      internalServerDBPath          (_internalServerDBPath),
      internalServerMysqlAdminCmd   (_internalServerMysqlAdminCmd),
      internalServerMysqlServCmd    (_internalServerMysqlServCmd),
      internalServerMysqlInitCmd    (_internalServerMysqlInitCmd)
{
}

DbEngineParameters::DbEngineParameters(const QUrl& url)
    : databaseType          (QUrlQuery(url).queryItemValue(QLatin1String("databaseType"))),
      databaseNameCore      (QUrlQuery(url).queryItemValue(QLatin1String("databaseNameCore"))),
      connectOptions        (QUrlQuery(url).queryItemValue(QLatin1String("connectOptions"))),
      hostName              (QUrlQuery(url).queryItemValue(QLatin1String("hostName"))),
      port                  (-1),
      walMode               (false),
      internalServer        (false),
      databaseNameThumbnails(QUrlQuery(url).queryItemValue(QLatin1String("databaseNameThumbnails"))),
      databaseNameFace      (QUrlQuery(url).queryItemValue(QLatin1String("databaseNameFace"))),
      databaseNameSimilarity(QUrlQuery(url).queryItemValue(QLatin1String("databaseNameSimilarity")))
{
    QString queryPort    = QUrlQuery(url).queryItemValue(QLatin1String("port"));

    if (!queryPort.isNull())
    {
        port = queryPort.toInt();
    }

    QString queryWalMode = QUrlQuery(url).queryItemValue(QLatin1String("walMode"));

    if (!queryWalMode.isNull())
    {
        walMode = (queryWalMode == QLatin1String("true"));
    }

#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)

    QString queryServer  = QUrlQuery(url).queryItemValue(QLatin1String("internalServer"));

    if (!queryServer.isNull())
    {
        internalServer = (queryServer == QLatin1String("true"));
    }

    queryServer          = QUrlQuery(url).queryItemValue(QLatin1String("internalServerPath"));

    if (!queryServer.isNull())
    {
        internalServerDBPath = QUrlQuery(url).queryItemValue(QLatin1String("internalServerPath"));
    }
    else
    {
        internalServerDBPath = internalServerPrivatePath();
    }

    internalServerMysqlAdminCmd = QUrlQuery(url).queryItemValue(QLatin1String("internalServerMysqlAdminCmd"));
    internalServerMysqlServCmd  = QUrlQuery(url).queryItemValue(QLatin1String("internalServerMysqlServCmd"));
    internalServerMysqlInitCmd  = QUrlQuery(url).queryItemValue(QLatin1String("internalServerMysqlInitCmd"));

#else

    internalServer = false;

#endif

    userName       = QUrlQuery(url).queryItemValue(QLatin1String("userName"));
    password       = QUrlQuery(url).queryItemValue(QLatin1String("password"));
}

void DbEngineParameters::insertInUrl(QUrl& url) const
{
    removeFromUrl(url);

    QUrlQuery q(url);
    q.addQueryItem(QLatin1String("databaseType"),           databaseType);
    q.addQueryItem(QLatin1String("databaseNameCore"),       databaseNameCore);
    q.addQueryItem(QLatin1String("databaseNameThumbnails"), databaseNameThumbnails);
    q.addQueryItem(QLatin1String("databaseNameFace"),       databaseNameFace);
    q.addQueryItem(QLatin1String("databaseNameSimilarity"), databaseNameSimilarity);

    if (!connectOptions.isNull())
    {
        q.addQueryItem(QLatin1String("connectOptions"), connectOptions);
    }

    if (!hostName.isNull())
    {
        q.addQueryItem(QLatin1String("hostName"), hostName);
    }

    if (port != -1)
    {
        q.addQueryItem(QLatin1String("port"), QString::number(port));
    }

    if (walMode)
    {
        q.addQueryItem(QLatin1String("walMode"), QLatin1String("true"));
    }

    if (internalServer)
    {
        q.addQueryItem(QLatin1String("internalServer"),              QLatin1String("true"));
        q.addQueryItem(QLatin1String("internalServerPath"),          internalServerDBPath);
        q.addQueryItem(QLatin1String("internalServerMysqlAdminCmd"), internalServerMysqlAdminCmd);
        q.addQueryItem(QLatin1String("internalServerMysqlServCmd"),  internalServerMysqlServCmd);
        q.addQueryItem(QLatin1String("internalServerMysqlInitCmd"),  internalServerMysqlInitCmd);
    }

    if (!userName.isNull())
    {
        q.addQueryItem(QLatin1String("userName"), userName);
    }

    if (!password.isNull())
    {
        q.addQueryItem(QLatin1String("password"), password);
    }

    url.setQuery(q);
}

void DbEngineParameters::removeFromUrl(QUrl& url)
{
    QUrlQuery q(url);

    q.removeQueryItem(QLatin1String("databaseType"));
    q.removeQueryItem(QLatin1String("databaseNameCore"));
    q.removeQueryItem(QLatin1String("databaseNameThumbnails"));
    q.removeQueryItem(QLatin1String("databaseNameFace"));
    q.removeQueryItem(QLatin1String("databaseNameSimilarity"));
    q.removeQueryItem(QLatin1String("connectOptions"));
    q.removeQueryItem(QLatin1String("hostName"));
    q.removeQueryItem(QLatin1String("port"));
    q.removeQueryItem(QLatin1String("walMode"));
    q.removeQueryItem(QLatin1String("internalServer"));
    q.removeQueryItem(QLatin1String("internalServerPath"));
    q.removeQueryItem(QLatin1String("internalServerMysqlAdminCmd"));
    q.removeQueryItem(QLatin1String("internalServerMysqlServCmd"));
    q.removeQueryItem(QLatin1String("internalServerMysqlInitCmd"));
    q.removeQueryItem(QLatin1String("userName"));
    q.removeQueryItem(QLatin1String("password"));

    url.setQuery(q);
}

bool DbEngineParameters::operator==(const DbEngineParameters& other) const
{
    return (
            (databaseType                == other.databaseType)                &&
            (databaseNameCore            == other.databaseNameCore)            &&
            (databaseNameThumbnails      == other.databaseNameThumbnails)      &&
            (databaseNameFace            == other.databaseNameFace)            &&
            (databaseNameSimilarity      == other.databaseNameSimilarity)      &&
            (connectOptions              == other.connectOptions)              &&
            (hostName                    == other.hostName)                    &&
            (port                        == other.port)                        &&
            (walMode                     == other.walMode)                     &&
            (internalServer              == other.internalServer)              &&
            (internalServerDBPath        == other.internalServerDBPath)        &&
            (internalServerMysqlAdminCmd == other.internalServerMysqlAdminCmd) &&
            (internalServerMysqlServCmd  == other.internalServerMysqlServCmd)  &&
            (internalServerMysqlInitCmd  == other.internalServerMysqlInitCmd)  &&
            (userName                    == other.userName)                    &&
            (password                    == other.password)
           );
}

bool DbEngineParameters::operator!=(const DbEngineParameters& other) const
{
    return (!operator == (other));
}

bool DbEngineParameters::isValid() const
{
    if (isSQLite())
    {
        return !databaseNameCore.isEmpty();
    }

    return false;
}

bool DbEngineParameters::isSQLite() const
{
    return (databaseType == QLatin1String("QSQLITE"));
}

bool DbEngineParameters::isMySQL() const
{
    return (databaseType == QLatin1String("QMYSQL"));
}

QString DbEngineParameters::SQLiteDatabaseType()
{
    return QLatin1String("QSQLITE");
}

QString DbEngineParameters::MySQLDatabaseType()
{
    return QLatin1String("QMYSQL");
}

QString DbEngineParameters::SQLiteDatabaseFile() const
{
    if (isSQLite())
    {
        return databaseNameCore;
    }

    return QString();
}

QByteArray DbEngineParameters::hash() const
{
    QCryptographicHash md5(QCryptographicHash::Md5);

    md5.addData(databaseType.toUtf8());
    md5.addData(databaseNameCore.toUtf8());
    md5.addData(databaseNameThumbnails.toUtf8());
    md5.addData(databaseNameFace.toUtf8());
    md5.addData(databaseNameSimilarity.toUtf8());
    md5.addData(connectOptions.toUtf8());
    md5.addData(hostName.toUtf8());
    md5.addData((const char*)&port, sizeof(int));
    md5.addData(userName.toUtf8());
    md5.addData(password.toUtf8());
    md5.addData((const char*)&internalServer, sizeof(bool));
    md5.addData(internalServerDBPath.toUtf8());

    return md5.result().toHex();
}

DbEngineParameters DbEngineParameters::parametersFromConfig(const QString& configGroup)
{
    DbEngineParameters parameters;
    parameters.readFromConfig(configGroup);

    return parameters;
}

void DbEngineParameters::readFromConfig(const QString& configGroup)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group;

    if (configGroup.isNull())
    {
        group = config->group(configGroupDatabase);
    }
    else
    {
        group = config->group(configGroup);
    }

    databaseType = group.readEntry(configDatabaseType, QString());

    if (isSQLite())     // see bug #267131
    {
        databaseNameCore        = group.readPathEntry(configDatabaseName,                    QString());
        databaseNameThumbnails  = group.readPathEntry(configDatabaseNameThumbnails,          QString());
        databaseNameFace        = group.readPathEntry(configDatabaseNameFace,                QString());
        databaseNameSimilarity  = group.readPathEntry(configDatabaseNameSimilarity,          QString());
        walMode                 = group.readEntry(configDatabaseWALMode,                     false);
    }
    else
    {
        databaseNameCore        = group.readEntry(configDatabaseName,                        QString());
        databaseNameThumbnails  = group.readEntry(configDatabaseNameThumbnails,              QString());
        databaseNameFace        = group.readEntry(configDatabaseNameFace,                    QString());
        databaseNameSimilarity  = group.readEntry(configDatabaseNameSimilarity,              QString());
    }

    hostName                    = group.readEntry(configDatabaseHostName,                    QString());
    port                        = group.readEntry(configDatabasePort,                        -1);
    userName                    = group.readEntry(configDatabaseUsername,                    QString());

    // Non encrypted password for compatibility.

    password                    = group.readEntry(configDatabasePassword,                    QString());

    if (password.isEmpty())
    {
        password                = group.readEntry(configDatabaseEncryptedPassword,           QString());

        if (!password.isEmpty())
        {
            O0SimpleCrypt crypto(QCryptographicHash::hash(configDatabaseEncryptedPassword,   QCryptographicHash::Sha1).toULongLong());
            password = crypto.decryptToString(password);
        }
    }

    connectOptions              = group.readEntry(configDatabaseConnectOptions,              QString());

#if defined(HAVE_MYSQLSUPPORT) && defined(HAVE_INTERNALMYSQL)

    internalServer              = group.readEntry(configInternalDatabaseServer,              false);
    internalServerDBPath        = group.readEntry(configInternalDatabaseServerPath,          internalServerPrivatePath());
    internalServerMysqlAdminCmd = group.readEntry(configInternalDatabaseServerMysqlAdminCmd, defaultMysqlAdminCmd());
    internalServerMysqlServCmd  = group.readEntry(configInternalDatabaseServerMysqlServCmd,  defaultMysqlServerCmd());
    internalServerMysqlInitCmd  = group.readEntry(configInternalDatabaseServerMysqlInitCmd,  defaultMysqlInitCmd());

#else

    internalServer             = false;

#endif

    if (isSQLite() && !databaseNameCore.isNull())
    {
        QString orgName = databaseNameCore;
        setCoreDatabasePath(orgName);
        setThumbsDatabasePath(orgName);
        setFaceDatabasePath(orgName);
        setSimilarityDatabasePath(orgName);
    }
}

void DbEngineParameters::setInternalServerPath(const QString& path)
{
    internalServerDBPath = path;
}

QString DbEngineParameters::internalServerPath() const
{
    QFileInfo fileInfo(internalServerDBPath);

    return QDir::cleanPath(fileInfo.filePath());
}

void DbEngineParameters::setCoreDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        databaseNameCore = coreDatabaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseNameCore = folderOrFileOrName;
    }
}

void DbEngineParameters::setThumbsDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        databaseNameThumbnails = thumbnailDatabaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseNameThumbnails = folderOrFileOrName;
    }
}

void DbEngineParameters::setFaceDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        databaseNameFace = faceDatabaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseNameFace = folderOrFileOrName;
    }
}

void DbEngineParameters::setSimilarityDatabasePath(const QString& folderOrFileOrName)
{
    if (isSQLite())
    {
        databaseNameSimilarity = similarityDatabaseFileSQLite(folderOrFileOrName);
    }
    else
    {
        databaseNameSimilarity = folderOrFileOrName;
    }
}

QString DbEngineParameters::coreDatabaseFileSQLite(const QString& folderOrFile)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return (QDir::cleanPath(fileInfo.filePath() + QLatin1Char('/') + QLatin1String(digikam4db)));
    }

    return QDir::cleanPath(folderOrFile);
}

QString DbEngineParameters::thumbnailDatabaseFileSQLite(const QString& folderOrFile)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return (QDir::cleanPath(fileInfo.filePath() + QLatin1Char('/') + QLatin1String(thumbnails_digikamdb)));
    }

    return QDir::cleanPath(folderOrFile);
}

QString DbEngineParameters::faceDatabaseFileSQLite(const QString& folderOrFile)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return (QDir::cleanPath(fileInfo.filePath() + QLatin1Char('/') + QLatin1String(face_digikamdb)));
    }

    return QDir::cleanPath(folderOrFile);
}

QString DbEngineParameters::similarityDatabaseFileSQLite(const QString& folderOrFile)
{
    QFileInfo fileInfo(folderOrFile);

    if (fileInfo.isDir())
    {
        return (QDir::cleanPath(fileInfo.filePath() + QLatin1Char('/') + QLatin1String(similarity_digikamdb)));
    }

    return QDir::cleanPath(folderOrFile);
}

void DbEngineParameters::legacyAndDefaultChecks(const QString& suggestedPath)
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();

    // Additional semantic checks for the database section.
    // If the internal server should be started, then the connection options must be reset

    if ((databaseType == QLatin1String("QMYSQL")) && internalServer)
    {
        const QString miscDir      = internalServerPrivatePath() + QLatin1String("db_misc");
        databaseNameCore           = QLatin1String("digikam");
        databaseNameThumbnails     = QLatin1String("digikam");
        databaseNameFace           = QLatin1String("digikam");
        databaseNameSimilarity     = QLatin1String("digikam");
        internalServer             = true;
        userName                   = QLatin1String("root");
        password.clear();

#ifdef Q_OS_WIN

        hostName                   = QLatin1String("localhost");
        port                       = 3307;
        connectOptions.clear();

#else

        hostName.clear();
        port                       = -1;
        connectOptions             = QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir);

#endif

    }

    if (databaseType.isEmpty())
    {
        // Empty 1.3 config: migration from older versions

        KConfigGroup group = config->group("Album Settings");

        QString databaseFilePath;

        if      (group.hasKey(configDatabaseFilePathEntry))
        {
            // 1.0 - 1.2 style database file path?

            databaseFilePath = group.readEntry(configDatabaseFilePathEntry, QString());
        }
        else if (group.hasKey(configAlbumPathEntry))
        {
            // <= 0.9 style album path entry?

            databaseFilePath = group.readEntry(configAlbumPathEntry, QString());
        }
        else if (!suggestedPath.isNull())
        {
            databaseFilePath = suggestedPath;
        }

        if (!databaseFilePath.isEmpty())
        {
            *this = parametersForSQLite(coreDatabaseFileSQLite(databaseFilePath));
        }

        // Be aware that schema updating from version <= 0.9 requires reading the "Album Path", so do not remove it here
    }
}

void DbEngineParameters::removeLegacyConfig()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Album Settings");

    if (group.hasKey(configDatabaseFilePathEntry))
    {
        group.deleteEntry(configDatabaseFilePathEntry);
    }

    if (group.hasKey(configAlbumPathEntry))
    {
        group.deleteEntry(configAlbumPathEntry);
    }
}

void DbEngineParameters::writeToConfig(const QString& configGroup) const
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group;

    if (configGroup.isNull())
    {
        group = config->group(configGroupDatabase);
    }
    else
    {
        group = config->group(configGroup);
    }

    QString dbName           = getCoreDatabaseNameOrDir();
    QString dbNameThumbs     = getThumbsDatabaseNameOrDir();
    QString dbNameFace       = getFaceDatabaseNameOrDir();
    QString dbNameSimilarity = getSimilarityDatabaseNameOrDir();

    group.writeEntry(configDatabaseType,                        databaseType);
    group.writeEntry(configDatabaseName,                        dbName);
    group.writeEntry(configDatabaseNameThumbnails,              dbNameThumbs);
    group.writeEntry(configDatabaseNameFace,                    dbNameFace);
    group.writeEntry(configDatabaseNameSimilarity,              dbNameSimilarity);
    group.writeEntry(configDatabaseHostName,                    hostName);
    group.writeEntry(configDatabasePort,                        port);
    group.writeEntry(configDatabaseWALMode,                     walMode);
    group.writeEntry(configDatabaseUsername,                    userName);

    O0SimpleCrypt crypto(QCryptographicHash::hash(configDatabaseEncryptedPassword, QCryptographicHash::Sha1).toULongLong());
    group.writeEntry(configDatabaseEncryptedPassword,           crypto.encryptToString(password));

    group.writeEntry(configDatabaseConnectOptions,              connectOptions);
    group.writeEntry(configInternalDatabaseServer,              internalServer);
    group.writeEntry(configInternalDatabaseServerPath,          internalServerDBPath);
    group.writeEntry(configInternalDatabaseServerMysqlAdminCmd, internalServerMysqlAdminCmd);
    group.writeEntry(configInternalDatabaseServerMysqlServCmd,  internalServerMysqlServCmd);
    group.writeEntry(configInternalDatabaseServerMysqlInitCmd,  internalServerMysqlInitCmd);

    group.deleteEntry(configDatabasePassword);  // Remove non encrypted password
}

QString DbEngineParameters::getCoreDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return coreDatabaseDirectorySQLite(databaseNameCore);
    }

    return databaseNameCore;
}

QString DbEngineParameters::getThumbsDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return thumbnailDatabaseDirectorySQLite(databaseNameThumbnails);
    }

    return databaseNameThumbnails;
}

QString DbEngineParameters::getFaceDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return faceDatabaseDirectorySQLite(databaseNameFace);
    }

    return databaseNameFace;
}

QString DbEngineParameters::getSimilarityDatabaseNameOrDir() const
{
    if (isSQLite())
    {
        return similarityDatabaseDirectorySQLite(databaseNameSimilarity);
    }

    return databaseNameSimilarity;
}

QString DbEngineParameters::coreDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(QLatin1String(digikam4db)))
    {
        QString chopped(path);
        chopped.chop(QString(QLatin1String(digikam4db)).length());

        return chopped;
    }

    return path;
}

QString DbEngineParameters::thumbnailDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(QLatin1String(thumbnails_digikamdb)))
    {
        QString chopped(path);
        chopped.chop(QString(QLatin1String(thumbnails_digikamdb)).length());

        return chopped;
    }

    return path;
}

QString DbEngineParameters::faceDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(QLatin1String(face_digikamdb)))
    {
        QString chopped(path);
        chopped.chop(QString(QLatin1String(face_digikamdb)).length());

        return chopped;
    }

    return path;
}

QString DbEngineParameters::similarityDatabaseDirectorySQLite(const QString& path)
{
    if (path.endsWith(QLatin1String(similarity_digikamdb)))
    {
        QString chopped(path);
        chopped.chop(QString(QLatin1String(similarity_digikamdb)).length());

        return chopped;
    }

    return path;
}

DbEngineParameters DbEngineParameters::defaultParameters(const QString& databaseType)
{
    DbEngineParameters parameters;

    // only the database name is needed

    DbEngineConfigSettings config          = DbEngineConfig::element(databaseType);
    parameters.databaseType                = databaseType;
    parameters.databaseNameCore            = config.databaseName;
    parameters.databaseNameThumbnails      = config.databaseName;
    parameters.databaseNameFace            = config.databaseName;
    parameters.databaseNameSimilarity      = config.databaseName;
    parameters.userName                    = config.userName;
    parameters.password                    = config.password;
    parameters.walMode                     = false;
    parameters.internalServer              = (databaseType == QLatin1String("QMYSQL"));
    parameters.internalServerDBPath        = (databaseType == QLatin1String("QMYSQL")) ? internalServerPrivatePath() : QString();
    parameters.internalServerMysqlAdminCmd = (databaseType == QLatin1String("QMYSQL")) ? defaultMysqlAdminCmd()      : QString();
    parameters.internalServerMysqlServCmd  = (databaseType == QLatin1String("QMYSQL")) ? defaultMysqlServerCmd()     : QString();
    parameters.internalServerMysqlInitCmd  = (databaseType == QLatin1String("QMYSQL")) ? defaultMysqlInitCmd()       : QString();

    QString hostName                       = config.hostName;
    QString port                           = config.port;
    QString connectOptions                 = config.connectOptions;

#ifdef Q_OS_WIN

    hostName.replace(QLatin1String("$$DBHOSTNAME$$"),      (databaseType == QLatin1String("QMYSQL"))
                                                           ? QLatin1String("localhost")
                                                           : QString());

    port.replace(QLatin1String("$$DBPORT$$"),              (databaseType == QLatin1String("QMYSQL"))
                                                           ? QLatin1String("3307")
                                                           : QLatin1String("-1"));

    connectOptions.replace(QLatin1String("$$DBOPTIONS$$"), QString());

#else

    hostName.replace(QLatin1String("$$DBHOSTNAME$$"),      QString());

    port.replace(QLatin1String("$$DBPORT$$"),              QLatin1String("-1"));

    const QString miscDir                 = internalServerPrivatePath() + QLatin1String("db_misc");
    connectOptions.replace(QLatin1String("$$DBOPTIONS$$"), (databaseType == QLatin1String("QMYSQL"))
                                                           ? QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(miscDir)
                                                           : QString());
#endif

    parameters.hostName                   = hostName;
    parameters.port                       = port.toInt();
    parameters.connectOptions             = connectOptions;

    qCDebug(DIGIKAM_DBENGINE_LOG) << "ConnectOptions " << parameters.connectOptions;

    return parameters;
}

DbEngineParameters DbEngineParameters::thumbnailParameters() const
{
    DbEngineParameters params = *this;
    params.databaseNameCore   = databaseNameThumbnails;

    return params;
}

DbEngineParameters DbEngineParameters::faceParameters() const
{
    DbEngineParameters params = *this;
    params.databaseNameCore   = databaseNameFace;

    return params;
}

DbEngineParameters DbEngineParameters::similarityParameters() const
{
    DbEngineParameters params = *this;
    params.databaseNameCore   = databaseNameSimilarity;

    return params;
}

DbEngineParameters DbEngineParameters::parametersForSQLite(const QString& databaseFile)
{
    // only the database name is needed

    DbEngineParameters params(QLatin1String("QSQLITE"), databaseFile);
    params.setCoreDatabasePath(databaseFile);
    params.setThumbsDatabasePath(params.getCoreDatabaseNameOrDir());
    params.setFaceDatabasePath(params.getCoreDatabaseNameOrDir());
    params.setSimilarityDatabasePath(params.getCoreDatabaseNameOrDir());

    return params;
}

DbEngineParameters DbEngineParameters::parametersForSQLiteDefaultFile(const QString& directory)
{
    return parametersForSQLite(QDir::cleanPath(directory + QLatin1Char('/') + QLatin1String(digikam4db)));
}

QString DbEngineParameters::defaultMysqlServerCmd()
{
    return QLatin1String("mysqld");             // For Linux, Windows and OSX
}

QString DbEngineParameters::defaultMysqlAdminCmd()
{
    return QLatin1String("mysqladmin");         // For Linux, Windows and OSX
}

QString DbEngineParameters::defaultMysqlInitCmd()
{
    return QLatin1String("mysql_install_db");   // For Linux, Windows and OSX
}

// --------------------------------

QDebug operator<<(QDebug dbg, const DbEngineParameters& p)
{
    dbg.nospace() << "Database Parameters:"                                                                  << QT_ENDL;
    dbg.nospace() << "   Type:                      " << p.databaseType                                      << QT_ENDL;
    dbg.nospace() << "   DB Core Name:              " << p.databaseNameCore                                  << QT_ENDL;
    dbg.nospace() << "   DB Thumbs Name:            " << p.databaseNameThumbnails                            << QT_ENDL;
    dbg.nospace() << "   DB Face Name:              " << p.databaseNameFace                                  << QT_ENDL;
    dbg.nospace() << "   DB Similarity Name:        " << p.databaseNameSimilarity                            << QT_ENDL;
    dbg.nospace() << "   Connect Options:           " << p.connectOptions                                    << QT_ENDL;
    dbg.nospace() << "   Host Name:                 " << p.hostName                                          << QT_ENDL;
    dbg.nospace() << "   Host Port:                 " << p.port                                              << QT_ENDL;
    dbg.nospace() << "   WAL Mode:                  " << p.walMode                                           << QT_ENDL;
    dbg.nospace() << "   Internal Server:           " << p.internalServer                                    << QT_ENDL;
    dbg.nospace() << "   Internal Server Path:      " << p.internalServerDBPath                              << QT_ENDL;
    dbg.nospace() << "   Internal Server Admin Cmd: " << p.internalServerMysqlAdminCmd                       << QT_ENDL;
    dbg.nospace() << "   Internal Server Serv Cmd:  " << p.internalServerMysqlServCmd                        << QT_ENDL;
    dbg.nospace() << "   Internal Server Init Cmd:  " << p.internalServerMysqlInitCmd                        << QT_ENDL;
    dbg.nospace() << "   Username:                  " << p.userName                                          << QT_ENDL;
    dbg.nospace() << "   Password:                  " << QString().fill(QLatin1Char('X'), p.password.size()) << QT_ENDL;

    return dbg.space();
}

} // namespace Digikam
