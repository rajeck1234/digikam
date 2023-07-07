/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-07-28
 * Description : main program from digiKam
 *
 * SPDX-FileCopyrightText: 2002-2006 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikam_config.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QString>
#include <QFileInfo>
#include <QSettings>
#include <QStringList>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QApplication>
#include <QImageReader>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QCommandLineOption>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>
#include <kaboutdata.h>

// ImageMagick includes

#ifdef HAVE_IMAGE_MAGICK

// Pragma directives to reduce warnings from ImageMagick header files.
#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic push
#       pragma GCC diagnostic ignored "-Wignored-qualifiers"
#       pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic push
#       pragma clang diagnostic ignored "-Wignored-qualifiers"
#       pragma clang diagnostic ignored "-Wkeyword-macro"
#   endif

#   include <Magick++.h>
using namespace Magick;

// Restore warnings
#   if !defined(Q_OS_DARWIN) && defined(Q_CC_GNU)
#       pragma GCC diagnostic pop
#   endif

#   if defined(Q_CC_CLANG)
#       pragma clang diagnostic pop
#   endif

#endif // HAVE_IMAGE_MAGICK

// Local includes

#include "digikam_debug.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "systemsettings.h"
#include "metaengine.h"
#include "dmessagebox.h"
#include "albummanager.h"
#include "firstrundlg.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "daboutdata.h"
#include "dbengineparameters.h"
#include "digikamapp.h"
#include "scancontroller.h"
#include "coredbaccess.h"
#include "thumbsdbaccess.h"
#include "facedbaccess.h"
#include "dxmlguiwindow.h"
#include "applicationsettings.h"
#include "similaritydbaccess.h"
#include "databaseserverstarter.h"
#include "filesdownloader.h"
#include "dfileoperations.h"

#ifdef Q_OS_WIN
#   include <windows.h>
#   include <shellapi.h>
#   include <objbase.h>
#endif

#if defined Q_OS_WIN
#   define MAIN_EXPORT __declspec(dllexport)
#   define MAIN_FN digikam_main
#else
#   define MAIN_EXPORT
#   define MAIN_FN main
#endif

using namespace Digikam;

extern "C" MAIN_EXPORT int MAIN_FN(int argc, char** argv)
{
    SystemSettings system(QLatin1String("digikam"));

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))

    // These settings has no effect with Qt6 (always enabled)

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps,
                                   system.useHighDpiPixmaps);


    if (system.useHighDpiScaling)
    {
        QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    }
    else
    {
        QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);
    }

#else

    QImageReader::setAllocationLimit(0);

#endif

    // OpenCV crash with face engine with OpenCL support
    // https://bugs.kde.org/show_bug.cgi?id=423632
    // https://bugs.kde.org/show_bug.cgi?id=426175

    // When analyzing with Heaptrack it was found
    // that a big memory leak is created in
    // libpocl when OpenCL is active.

    if (system.disableOpenCL)
    {
        qputenv("OPENCV_OPENCL_RUNTIME", "disabled");
        qputenv("OPENCV_OPENCL_DEVICE",  "disabled");
    }

#ifdef HAVE_QWEBENGINE

    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);

#endif

    QApplication app(argc, argv);

    digikamSetDebugFilterRules(system.enableLogging);

    tryInitDrMingw();

#ifdef HAVE_IMAGE_MAGICK

    InitializeMagick(nullptr);

#endif

#ifdef Q_OS_MACOS

    // See bug #461734
    app.setAttribute(Qt::AA_DontShowIconsInMenus, true);

#endif

    // if we have some local breeze icon resource, prefer it

    DXmlGuiWindow::setupIconTheme();

    KLocalizedString::setApplicationDomain("digikam");

    KAboutData aboutData(QLatin1String("digikam"), // component name
                         i18n("digiKam"),          // display name
                         digiKamVersion());

    aboutData.setShortDescription(QString::fromUtf8("%1 - %2").arg(DAboutData::digiKamSlogan())
                                                              .arg(DAboutData::digiKamFamily()));
    aboutData.setLicense(KAboutLicense::GPL);
    aboutData.setCopyrightStatement(DAboutData::copyright());
    aboutData.setOtherText(additionalInformation());
    aboutData.setHomepage(DAboutData::webProjectUrl().url());
    aboutData.setTranslator(i18nc("NAME OF TRANSLATORS", "Your names"),
                            i18nc("EMAIL OF TRANSLATORS", "Your emails"));

    DAboutData::authorsRegistration(aboutData);

    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    aboutData.setupCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("download-from"),
                                        i18n("Open camera dialog at \"path\""),
                                        QLatin1String("path")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("download-from-udi"),
                                        i18n("Open camera dialog for the device with Solid UDI \"udi\""),
                                        QLatin1String("udi")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("detect-camera"),
                                        i18n("Automatically detect and open a connected gphoto2 camera")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("database-directory"),
                                        i18n("Start digikam with the SQLite database file found in the directory \"dir\""),
                                        QLatin1String("dir")));
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("config"),
                                        i18n("Start digikam with the configuration file \"config\""),
                                        QLatin1String("config")));

    parser.process(app);
    aboutData.processCommandLine(&parser);

    setupKSycocaDatabaseFile();

    // See bug #438701

    installQtTranslationFiles(app);

    // ---

    MetaEngine::initializeExiv2();

    // Force to use application icon for non plasma desktop as Unity for ex.

    QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("digikam"), app.windowIcon()));

#ifdef Q_OS_WIN

    if (QSysInfo::currentCpuArchitecture().contains(QLatin1String("64")) &&
        !QSysInfo::buildCpuArchitecture().contains(QLatin1String("64")))
    {
        QMessageBox::critical(qApp->activeWindow(),
                              qApp->applicationName(),
                              i18n("<p>You are running digiKam as a 32-bit version on a 64-bit Windows.</p>"
                                   "<p>Please install the 64-bit version of digiKam to get "
                                   "a better experience with digiKam.</p>"));
    }

    QString appPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QUrl    appUrl  = QUrl::fromLocalFile(appPath).adjusted(QUrl::RemoveFilename);
    appUrl.setPath(appUrl.path() + QLatin1String("digikam/facesengine"));

    QFileInfo appInfo(appUrl.toLocalFile());

    if (appInfo.exists() && appInfo.isDir())
    {
        QString appLocalPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        QUrl    appLocalUrl  = QUrl::fromLocalFile(appLocalPath).adjusted(QUrl::RemoveFilename);
        appLocalUrl.setPath(appLocalUrl.path() + QLatin1String("digikam"));

        qCDebug(DIGIKAM_GENERAL_LOG) << "Copy faces engine data from"
                                     << appUrl.toLocalFile() << "to"
                                     << appLocalUrl.toLocalFile();

        if (DFileOperations::copyFolderRecursively(appUrl.toLocalFile(), appLocalUrl.toLocalFile()))
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "Delete faces engine data from"
                                         << appUrl.toLocalFile();

            QDir rmAppPath(appUrl.toLocalFile());
            rmAppPath.removeRecursively();
        }
    }

#endif

    // Check if Qt database plugins are available.

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()) &&
        !QSqlDatabase::isDriverAvailable(DbEngineParameters::MySQLDatabaseType()))
    {
        if (QSqlDatabase::drivers().isEmpty())
        {
            QMessageBox::critical(qApp->activeWindow(),
                                  qApp->applicationName(),
                                  i18n("Run-time Qt SQLite or MySQL database plugin is not available. "
                                       "please install it.\n"
                                       "There is no database plugin installed on your computer."));
        }
        else
        {
            DMessageBox::showInformationList(QMessageBox::Warning,
                                             qApp->activeWindow(),
                                             qApp->applicationName(),
                                             i18n("Run-time Qt SQLite or MySQL database plugin are not available. "
                                                  "Please install it.\n"
                                                  "Database plugins installed on your computer are listed below."),
                                             QSqlDatabase::drivers());
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "QT Sql drivers list: " << QSqlDatabase::drivers();

        return 1;
    }

    QString commandLineDBPath;

    if (parser.isSet(QLatin1String("database-directory")))
    {
        QDir commandLineDBDir(parser.value(QLatin1String("database-directory")));

        if (!commandLineDBDir.exists())
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << "The given database-directory does not exist or is not readable. Ignoring."
                                         << commandLineDBDir.absolutePath();
        }
        else
        {
            commandLineDBPath = commandLineDBDir.absolutePath();
        }
    }

    if (parser.isSet(QLatin1String("config")))
    {
        QString configFilename = parser.value(QLatin1String("config"));
        QFileInfo configFile(configFilename);

        if (configFile.isDir()       || !configFile.dir().exists() ||
            !configFile.isReadable() || !configFile.isWritable())
        {
            QMessageBox::critical(qApp->activeWindow(),
                                  qApp->applicationName(),
                                  QLatin1String("--config ") +
                                  configFilename             +
                                  i18n("<p>The given path for the config file "
                                       "is not valid. Either its parent "
                                       "directory does not exist, it is a "
                                       "directory itself or it cannot be read/"
                                       "written to.</p>"));
            qCDebug(DIGIKAM_GENERAL_LOG) << "Invalid path: --config"
                                         << configFilename;
            return 1;
        }
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("General Settings"));
    QString version           = group.readEntry(QLatin1String("Version"), QString());
    QString iconTheme         = group.readEntry(QLatin1String("Icon Theme"), QString());
    KConfigGroup mainConfig   = config->group(QLatin1String("Album Settings"));

    QString            firstAlbumPath;
    DbEngineParameters params;

    // Run the first run assistant if we have no or very old config

    if (!mainConfig.exists() || (version.startsWith(QLatin1String("0.5"))))
    {
        FirstRunDlg firstRun;
        firstRun.show();

        if (firstRun.exec() == QDialog::Rejected)
        {
            return 1;
        }

        // parameters are written to config

        firstAlbumPath = firstRun.firstAlbumPath();

        if (firstRun.getDbEngineParameters().isSQLite())
        {
            AlbumManager::checkDatabaseDirsAfterFirstRun(firstRun.getDbEngineParameters().getCoreDatabaseNameOrDir(), firstAlbumPath);
        }
    }

    if (!commandLineDBPath.isNull())
    {
        // command line option set?

        params = DbEngineParameters::parametersForSQLiteDefaultFile(commandLineDBPath);
        ApplicationSettings::instance()->setDatabaseDirSetAtCmd(true);
        ApplicationSettings::instance()->setDbEngineParameters(params);
    }
    else
    {
        params = DbEngineParameters::parametersFromConfig();
        params.legacyAndDefaultChecks(firstAlbumPath);

        // sync to config, for all first-run or upgrade situations

        params.writeToConfig();
        ApplicationSettings::instance()->setDbEngineParameters(params);
    }

    // initialize database

    if (!AlbumManager::instance()->setDatabase(params, !commandLineDBPath.isNull(), firstAlbumPath))
    {
        DatabaseServerStarter::instance()->stopServerManagerProcess();

        CoreDbAccess::cleanUpDatabase();
        ThumbsDbAccess::cleanUpDatabase();
        FaceDbAccess::cleanUpDatabase();
        SimilarityDbAccess::cleanUpDatabase();

        return 0;
    }

    if (!iconTheme.isEmpty())
    {
        QIcon::setThemeName(iconTheme);
    }

#ifdef Q_OS_WIN

    // Necessary to open native open with dialog on windows

    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

#endif

    // create main window

    DigikamApp* const digikam = new DigikamApp();

    // If application storage place in home directory to save customized XML settings files do not exist, create it,
    // else QFile will not able to create new files as well.

    if (!QFile::exists(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)))
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }

    // If application cache place in home directory to save cached files do not exist, create it.
    if (!QFile::exists(QStandardPaths::writableLocation(QStandardPaths::CacheLocation)))
    {
        QDir().mkpath(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    }

    // Bug #247175:
    // Add a connection to the destroyed() signal when the digiKam mainwindow has been
    // closed. This should prevent digiKam from staying open in the background.
    //
    // Right now this is the easiest and cleanest fix for the described problem, but we might re-think the
    // solution later on, just in case there are better ways to do it.

    QObject::connect(digikam, SIGNAL(destroyed(QObject*)),
                     &app, SLOT(quit()));

    digikam->restoreSession();
    digikam->show();

    if (!system.disableFaceEngine)
    {
        QPointer<FilesDownloader> floader = new FilesDownloader(qApp->activeWindow());

        if (!floader->checkDownloadFiles())
        {
            floader->startDownload();
        }

        delete floader;
    }

    if      (parser.isSet(QLatin1String("download-from")))
    {
        digikam->downloadFrom(parser.value(QLatin1String("download-from")));
    }
    else if (parser.isSet(QLatin1String("download-from-udi")))
    {
        digikam->downloadFromUdi(parser.value(QLatin1String("download-from-udi")));
    }
    else if (parser.isSet(QLatin1String("detect-camera")))
    {
        digikam->autoDetect();
    }

    int ret = app.exec();

    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();
    SimilarityDbAccess::cleanUpDatabase();

#ifdef Q_OS_WIN

    // Necessary to open native open with dialog on windows
    CoUninitialize();

#endif

#ifdef HAVE_IMAGE_MAGICK
#   if MagickLibVersion >= 0x693

    TerminateMagick();

#   endif
#endif

    return ret;
}
