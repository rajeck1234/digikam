/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Mysql internal database server
 *
 * SPDX-FileCopyrightText: 2009-2011 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2016      by Swati Lodha <swatilodha27 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databaseserver.h"

// Qt includes

#include <QStandardPaths>
#include <QApplication>
#include <QMessageBox>
#include <QTcpServer>
#include <QDateTime>
#include <QFileInfo>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QtGlobal>
#include <QPointer>
#include <QDir>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dbengineparameters.h"
#include "databaseserverstarter.h"

namespace Digikam
{

class Q_DECL_HIDDEN DatabaseServer::Private
{
public:

    explicit Private()
        : databaseProcess(nullptr),
          serverPort(3307)
    {
    }

    DbEngineParameters     params;
    QProcess*              databaseProcess;

    QString                internalDBName;
    QString                mysqlAdminPath;
    QString                mysqldInitPath;
    QString                mysqldCmd;
    QString                dataDir;
    QString                miscDir;
    QString                fileDataDir;
    QString                actualConfig;
    QString                globalConfig;

    int                    serverPort;
};

DatabaseServer::DatabaseServer(const DbEngineParameters& params, DatabaseServerStarter* const parent)
    : QThread(parent),
      d      (new Private)
{
    d->params = params;

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << d->params;

    QString defaultAkDir = DbEngineParameters::internalServerPrivatePath();
    QString dataDir;

    if (d->params.internalServerPath().isEmpty())
    {
        dataDir = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_data"));
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No internal server data path is given, we will use the default." << dataDir;
    }
    else
    {
        dataDir = QDir(d->params.internalServerPath()).absoluteFilePath(QLatin1String(".mysql.digikam/db_data"));
    }

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Internal Server data path:" << dataDir;

    d->internalDBName       = QLatin1String("digikam");
    d->mysqlAdminPath       = d->params.internalServerMysqlAdminCmd;
    d->mysqldInitPath       = d->params.internalServerMysqlInitCmd;
    d->mysqldCmd            = d->params.internalServerMysqlServCmd;
    d->dataDir              = dataDir;
    d->miscDir              = QDir(defaultAkDir).absoluteFilePath(QLatin1String("db_misc"));
    d->fileDataDir          = QDir(defaultAkDir).absoluteFilePath(QLatin1String("file_db_data"));
    d->actualConfig         = QDir(defaultAkDir).absoluteFilePath(QLatin1String("mysql.conf"));
    d->globalConfig         = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                     QLatin1String("digikam/database/mysql-global.conf"));
    databaseServerStateEnum = started;
}

DatabaseServer::~DatabaseServer()
{
    delete d;
}

void DatabaseServer::run()
{
    quint64 runningTime = 0;
    int debugTime       = 0;
    int waitTime        = 1;

    // Loop to wait for stopping the server.

    do
    {
        if (!debugTime)
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Running" << runningTime << "seconds...";
            debugTime = 30;
        }

        QThread::sleep(waitTime);
        ++runningTime;
        --debugTime;
    }
    while (databaseServerStateEnum != stopped);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Shutting down database server";
    Q_EMIT done();
}

DatabaseServerError DatabaseServer::startDatabaseProcess()
{
    DatabaseServerError error;

    if (d->params.isMySQL())
    {
        error = startMysqlDatabaseProcess();
    }
    else
    {
        error = DatabaseServerError(DatabaseServerError::NotSupported,
                                    i18n("Database type is not supported."));
    }

    if     (error.getErrorType() == DatabaseServerError::StartError)
    {
        databaseServerStateEnum = notRunning;
    }
    else if (error.getErrorType() == DatabaseServerError::NotSupported)
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "This database type is not supported.";
        databaseServerStateEnum = notRunning;
    }
    else
    {
        databaseServerStateEnum = running;
    }

    return error;
}

void DatabaseServer::stopDatabaseProcess()
{
    if (!d->databaseProcess)
    {
        return;
    }

    QStringList mysqlShutDownArgs;
    mysqlShutDownArgs << QLatin1String("-u");
    mysqlShutDownArgs << QLatin1String("root");
    mysqlShutDownArgs << QLatin1String("shutdown");

#ifdef Q_OS_WIN

    mysqlShutDownArgs << QString::fromLatin1("--port=%1").arg(d->serverPort);

#else

    mysqlShutDownArgs << QString::fromLatin1("--socket=%1/mysql.socket").arg(d->miscDir);

#endif

    QProcess mysqlShutDownProcess;
    mysqlShutDownProcess.setProcessEnvironment(adjustedEnvironmentForAppImage());
    mysqlShutDownProcess.start(d->mysqlAdminPath, mysqlShutDownArgs);
    mysqlShutDownProcess.waitForFinished();

    if ((d->databaseProcess->state() == QProcess::Running) && !d->databaseProcess->waitForFinished())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database process will be killed now";
        d->databaseProcess->kill();
        d->databaseProcess->waitForFinished();
    }

    delete d->databaseProcess;
    d->databaseProcess      = nullptr;
    databaseServerStateEnum = stopped;

    wait();
}

bool DatabaseServer::isRunning() const
{
    if (!d->databaseProcess)
    {
        return false;
    }

    return (databaseServerStateEnum == running);
}

DatabaseServerError DatabaseServer::startMysqlDatabaseProcess()
{
    DatabaseServerError error = checkDatabaseDirs();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    error = initMysqlConfig();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    bool needUpgrade = checkAndRemoveMysqlLogs();

    error = createMysqlFiles();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    error = startMysqlServer();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    if (needUpgrade)
    {
        error = upgradeMysqlDatabase();

        if (error.getErrorType() != DatabaseServerError::NoErrors)
        {
            return error;
        }
    }

    error = initMysqlDatabase();

    if (error.getErrorType() != DatabaseServerError::NoErrors)
    {
        return error;
    }

    databaseServerStateEnum = running;

    return error;
}

DatabaseServerError DatabaseServer::checkDatabaseDirs() const
{
    DatabaseServerError result;

    if (d->mysqldCmd.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql server command set in configuration file!";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("No path to mysql server command set "
                                        "in configuration file."));
    }

    if (d->mysqldInitPath.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql initialization command set in configuration file!";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("No path to mysql initialization "
                                        "command set in configuration file."));
    }

    if (d->mysqlAdminPath.isEmpty())
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "No path to mysql administration command set in configuration file!";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("No path to mysql administration "
                                        "command set in configuration file."));
    }

    if (!QFile::exists(d->dataDir) && !QDir().mkpath(d->dataDir))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory "
                                            << d->dataDir;

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot create directory %1",
                                        QDir::toNativeSeparators(d->dataDir)));
    }

    if (!QFile::exists(d->miscDir) && !QDir().mkpath(d->miscDir))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory "
                                            << d->miscDir;

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot create directory %1",
                                        QDir::toNativeSeparators(d->miscDir)));
    }

    if (!QFile::exists(d->fileDataDir) && !QDir().mkpath(d->fileDataDir))
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot create directory "
                                            << d->fileDataDir;

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot create directory %1",
                                        QDir::toNativeSeparators(d->fileDataDir)));
    }

    return result;
}

DatabaseServerError DatabaseServer::initMysqlConfig() const
{
    DatabaseServerError result;

    QString localConfig = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                 QLatin1String("digikam/database/mysql-local.conf"));

    if (!d->globalConfig.isEmpty())
    {
        bool confUpdate       = false;
        bool confShouldUpdate = false;
        QFile actualFile(d->actualConfig);

        // Update actualconf only if either global or local is newer than actual
        // If actual does not yet exist it was initialised with datetime 0
        // so it will get updated too

        if ((QFileInfo(d->globalConfig).lastModified() > QFileInfo(actualFile).lastModified()) ||
            (QFileInfo(localConfig).lastModified()     > QFileInfo(actualFile).lastModified()))
        {
            confShouldUpdate = true;

            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "The mysql configuration is outdated,"
                                                << d->actualConfig
                                                << "will be updated.";

            QFile globalFile(d->globalConfig);
            QFile localFile(localConfig);

            if (globalFile.open(QFile::ReadOnly) && actualFile.open(QFile::WriteOnly))
            {
                actualFile.write(globalFile.readAll());
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Updated mysql configuration with" << d->globalConfig;

                if (!localConfig.isEmpty() && localFile.open(QFile::ReadOnly))
                {
                    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Updated mysql configuration with" << localConfig;
                    actualFile.write(localFile.readAll());
                    localFile.close();
                }

                globalFile.close();
                actualFile.close();

                confUpdate = true;
            }
        }

        // MySQL doesn't like world writeable config files (which makes sense), but
        // our config file somehow ends up being world-writable on some systems for no
        // apparent reason nevertheless, so fix that

        if      (confUpdate)
        {
            const QFile::Permissions allowedPerms = actualFile.permissions() &
                                                    (QFile::ReadOwner | QFile::WriteOwner |
                                                     QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther);

            if (allowedPerms != actualFile.permissions())
            {
                actualFile.setPermissions(allowedPerms);
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Fixed permissions of mysql configuration file.";
            }
        }
        else if (confShouldUpdate)
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Unable to create MySQL server configuration file."
                                                << "This means that either the default configuration file"
                                                << d->globalConfig
                                                << "was not readable or the target file"
                                                << d->actualConfig
                                                << "could not be written.";

            QString errorMsg = i18n("Unable to create MySQL server configuration file."
                                    "<p>This means that either the default configuration file</p>"
                                    "<p>%1</p>"
                                    "<p>was not readable or the target file</p>"
                                    "<p>%2</p>"
                                    "<p>could not be written.</p>",
                                    d->globalConfig,
                                    d->actualConfig);

            return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
        }
        else
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "The mysql configuration was already up-to-date:"
                                                << d->actualConfig;
        }
    }
    else
    {
        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Cannot find MySQL server default configuration (mysql-global.conf)";

        return DatabaseServerError(DatabaseServerError::StartError,
                                   i18n("Cannot find MySQL server default "
                                        "configuration (mysql-global.conf)."));
    }

    return result;
}

bool DatabaseServer::checkAndRemoveMysqlLogs() const
{
    // Move mysql error log file out of the way

    const QFileInfo errorLog(d->dataDir,
                             QLatin1String("mysql.err"));

    bool needUpgrade = false;

    if (errorLog.exists())
    {
        QFile logFile(errorLog.absoluteFilePath());
        QFile oldLogFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("mysql.err.old")));

        if (logFile.open(QFile::ReadOnly) && oldLogFile.open(QFile::Append))
        {
            QByteArray runMaria("run mariadb-upgrade");
            QByteArray runMysql("run mysql_upgrade");
            QByteArray ba = logFile.readAll();
            needUpgrade   = (ba.contains(runMaria) || ba.contains(runMysql));

            oldLogFile.write(ba);
            oldLogFile.close();
            logFile.close();
            logFile.remove();
        }
        else
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to open MySQL error log.";
        }
    }

    // Clear mysql ib_logfile's in case innodb_log_file_size option changed in last confUpdate

    //QFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("ib_logfile0"))).remove();
    //QFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("ib_logfile1"))).remove();

    return needUpgrade;
}

DatabaseServerError DatabaseServer::createMysqlFiles() const
{
    DatabaseServerError result;

    // Initialize the database

    if (!QFile(QDir(d->dataDir).absoluteFilePath(QLatin1String("mysql"))).exists())
    {
        // Synthesize the server initialization command line arguments

        QStringList mysqlInitCmdArgs;

#ifndef Q_OS_WIN

        mysqlInitCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--defaults-file=%1")
                                                     .arg(d->globalConfig));

#endif

#ifdef Q_OS_MACOS

        mysqlInitCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--basedir=%1/lib/mariadb/")
                                                     .arg(macOSBundlePrefix()));

#endif

        mysqlInitCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--datadir=%1")
                                                     .arg(d->dataDir));

        QProcess initProcess;
        initProcess.setProcessEnvironment(adjustedEnvironmentForAppImage());
        initProcess.start(d->mysqldInitPath, mysqlInitCmdArgs);

        qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database initializer:"
                                            << initProcess.program()
                                            << initProcess.arguments();

        if (!initProcess.waitForFinished() || (initProcess.exitCode() != 0))
        {
            return DatabaseServerError(DatabaseServerError::StartError,
                                       processErrorLog(&initProcess,
                                                       i18n("Could not start database initializer.")));
        }
    }

    return result;
}

DatabaseServerError DatabaseServer::startMysqlServer()
{
    DatabaseServerError result;

    // Synthesize the server command line arguments

    QStringList mysqldCmdArgs;
    mysqldCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--defaults-file=%1").arg(d->actualConfig))
                  << QDir::toNativeSeparators(QString::fromLatin1("--datadir=%1").arg(d->dataDir));

#ifdef Q_OS_MACOS

    mysqldCmdArgs << QDir::toNativeSeparators(QString::fromLatin1("--basedir=%1/lib/mariadb/")
                                              .arg(macOSBundlePrefix()));

#endif

#ifdef Q_OS_WIN

    QTcpServer* const server = new QTcpServer();

    if (!server->listen(QHostAddress::LocalHost, d->serverPort))
    {
        qCWarning(DIGIKAM_DATABASESERVER_LOG) << "Port 3307 not free for the MySQL server";

        server->listen(QHostAddress::LocalHost, 0);
        d->serverPort = server->serverPort();

        qCWarning(DIGIKAM_DATABASESERVER_LOG) << "Now use the free port:"  << d->serverPort;
    }

    server->close();
    delete server;

    mysqldCmdArgs << QLatin1String("--skip-networking=0")
                  << QString::fromLatin1("--port=%1").arg(d->serverPort);

#else

    mysqldCmdArgs << QString::fromLatin1("--socket=%1/mysql.socket").arg(d->miscDir);

#endif

    // Start the database server

    d->databaseProcess = new QProcess();
    d->databaseProcess->setProcessEnvironment(adjustedEnvironmentForAppImage());
    d->databaseProcess->start(d->mysqldCmd, mysqldCmdArgs);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database server:"
                                        << d->databaseProcess->program()
                                        << d->databaseProcess->arguments();

    if (!d->databaseProcess->waitForStarted() || (d->databaseProcess->exitCode() != 0))
    {
        QString errorMsg = processErrorLog(d->databaseProcess,
                                           i18n("Could not start database server."));

        delete d->databaseProcess;
        d->databaseProcess = nullptr;

        return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
    }

    return result;
}

DatabaseServerError DatabaseServer::initMysqlDatabase() const
{
    DatabaseServerError result;

    const QLatin1String initCon("initConnection");

    {
        QSqlDatabase db = QSqlDatabase::addDatabase(DbEngineParameters::MySQLDatabaseType(), initCon);

#ifdef Q_OS_WIN

        db.setHostName(QLatin1String("localhost"));
        db.setPort(d->serverPort);

#else

        db.setConnectOptions(QString::fromLatin1("UNIX_SOCKET=%1/mysql.socket").arg(d->miscDir));

#endif

        db.setUserName(QLatin1String("root"));

        // might not exist yet, then connecting to the actual db will fail

        db.setDatabaseName(QString());

        if (!db.isValid())
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Invalid database object during database server startup";

            return DatabaseServerError(DatabaseServerError::StartError,
                                       i18n("Invalid database object during database "
                                            "server startup"));
        }

        bool opened = false;
        bool exited = false;

        for (int i = 0 ; i < 120 ; ++i)
        {
            opened = db.open();

            if (opened)
            {
                break;
            }

            if (d->databaseProcess && d->databaseProcess->waitForFinished(500))
            {
                exited = true;
                break;
            }
        }

        if (!opened)
        {
            QString firstLine;
            QString errorMsg;

            if (exited)
            {
                errorMsg = processErrorLog(d->databaseProcess,
                                           i18n("Database process exited unexpectedly "
                                                "during initial connection."));
            }
            else
            {
                errorMsg = processErrorLog(d->databaseProcess,
                                           i18n("Could not connect to Database after "
                                                "trying for 60 seconds."));
            }

            return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
        }

        QSqlQuery query(db);

        if (!query.exec(QString::fromLatin1("USE %1;").arg(d->internalDBName)))
        {
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to use database"
                                                << d->internalDBName;
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"
                                                << query.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"
                                                << db.lastError().text();
            qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Trying to create database now";

            if (query.exec(QString::fromLatin1("CREATE DATABASE %1;").arg(d->internalDBName)))
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database was successfully created";
            }
            else
            {
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Failed to create database";
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Query error:"
                                                    << query.lastError().text();
                qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Database error:"
                                                    << db.lastError().text();

                QString  errorMsg = i18n("Failed to create database"
                                         "<p>Query error: %1</p>"
                                         "<p>Database error: %2</p>",
                                         query.lastError().text(),
                                         db.lastError().text());

                return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
            }

            // Make sure query is destroyed before we close the db

            db.close();
        }
    }

    QSqlDatabase::removeDatabase(initCon);

    return result;
}

DatabaseServerError DatabaseServer::upgradeMysqlDatabase()
{
    QApplication::restoreOverrideCursor();

    QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Information,
             qApp->applicationName(),
             i18n("The database is now upgraded to the current "
                  "server version, this can take a while."),
             QMessageBox::Ok | QMessageBox::Cancel,
             qApp->activeWindow());

    int msgResult = msgBox->exec();
    delete msgBox;

    QApplication::setOverrideCursor(Qt::WaitCursor);

    if (msgResult == QMessageBox::Cancel)
    {
        return DatabaseServerError();
    }

    // Synthesize the mysql upgrade command line arguments

    QStringList upgradeCmdArgs;

#ifdef Q_OS_WIN

    upgradeCmdArgs << QString::fromLatin1("--port=%1").arg(d->serverPort);

#else

    upgradeCmdArgs << QString::fromLatin1("--socket=%1/mysql.socket").arg(d->miscDir);

#endif

    // Start the upgrade ptogram

    QUrl upgradeUrl = QUrl::fromLocalFile(d->mysqlAdminPath).adjusted(QUrl::RemoveFilename);
    upgradeUrl.setPath(upgradeUrl.path() + QLatin1String("mysql_upgrade"));

    QProcess* const upgradeProcess = new QProcess();
    upgradeProcess->setProcessEnvironment(adjustedEnvironmentForAppImage());
    upgradeProcess->start(upgradeUrl.toLocalFile(), upgradeCmdArgs);

    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Upgrade database:"
                                        << upgradeProcess->program()
                                        << upgradeProcess->arguments();

    if (!upgradeProcess->waitForFinished(-1) || (upgradeProcess->exitCode() != 0))
    {
        QString errorMsg = processErrorLog(upgradeProcess,
                                           i18n("Could not upgrade database."));

        delete upgradeProcess;

        return DatabaseServerError(DatabaseServerError::StartError, errorMsg);
    }

    delete upgradeProcess;

    // Restart the database server.

    stopDatabaseProcess();

    return startMysqlServer();
}

QString DatabaseServer::getcurrentAccountUserName() const
{
    QString name = QString::fromUtf8(qgetenv("USER"));   // Linux and OSX

    if (name.isEmpty())
    {
        name = QString::fromUtf8(qgetenv("USERNAME"));   // Windows
    }

    return name;
}

QString DatabaseServer::processErrorLog(QProcess* const process, const QString& msg) const
{
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << msg;
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Executable:"
                                        << process->program();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Arguments:"
                                        << process->arguments().join(QLatin1String(", "));
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Stdout:"
                                        << process->readAllStandardOutput();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Stderr:"
                                        << process->readAllStandardError();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Exit code:"
                                        << process->exitCode();
    qCDebug(DIGIKAM_DATABASESERVER_LOG) << "Process error:"
                                        << process->errorString();

    return i18n("%1"
                "<p>Executable: %2</p>"
                "<p>Arguments: %3</p>"
                "<p>Process error: %4</p>",
                msg,
                process->program(),
                process->arguments().join(QLatin1String(", ")),
                process->errorString());
}

} // namespace Digikam
