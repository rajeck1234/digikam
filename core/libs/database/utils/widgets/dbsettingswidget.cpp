/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database settings widget
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbsettingswidget_p.h"

namespace Digikam
{

DatabaseSettingsWidget::DatabaseSettingsWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setupMainArea();
}

DatabaseSettingsWidget::~DatabaseSettingsWidget()
{
    delete d;
}

void DatabaseSettingsWidget::setupMainArea()
{
    QVBoxLayout* const layout       = new QVBoxLayout();
    setLayout(layout);

    // --------------------------------------------------------

    const int spacing = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    QGroupBox* const dbConfigBox    = new QGroupBox(i18n("Database Configuration"), this);
    QVBoxLayout* const vlay         = new QVBoxLayout(dbConfigBox);

    DHBox* const typeHbox           = new DHBox();
    QLabel* const databaseTypeLabel = new QLabel(typeHbox);
    d->dbType                       = new QComboBox(typeHbox);
    databaseTypeLabel->setText(i18n("Type:"));

    // --------- fill with default values ---------------------

    int dbTypeIdx               = 0;
    d->dbType->addItem(i18n("SQLite"),                        SQlite);
    d->dbTypeMap[SQlite]        = dbTypeIdx++;

#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL

    d->dbType->addItem(i18n("Mysql Internal"), MysqlInternal);
    d->dbTypeMap[MysqlInternal] = dbTypeIdx++;

#   endif

    d->dbType->addItem(i18n("MySQL Server"),   MysqlServer);
    d->dbTypeMap[MysqlServer]   = dbTypeIdx++;

#endif

    QString tip = i18n("<p>Select here the type of database backend.</p>"
                       "<p><b>SQlite</b> backend is for local database storage with a small or medium collection sizes. "
                       "It is the default and recommended backend for collections with less than 100K items.</p>");


#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL

    tip.append(i18n("<p><b>MySQL Internal</b> backend is for local database storage with huge collection sizes. "
                    "This backend is recommend for local collections with more than 100K items.</p>"));

#   endif

    tip.append(i18n("<p><b>MySQL Server</b> backend is a more robust solution especially for remote and shared database storage. "
                    "It is also more efficient to manage huge collection sizes with more than 100K items.</p>"));

#endif

    d->dbType->setToolTip(tip);

    // --------------------------------------------------------

    d->dbPathLabel = new QLabel(i18n("<p>Set here the location where the database files will be stored on your system. "
                                     "There are four databases: "
                                     "one for all collections properties, "
                                     "one to store compressed thumbnails, "
                                     "one to store faces recognition metadata, "
                                     "and one to store similarity fingerprints.<br/>"
                                     "Write access is required to be able to edit image properties.</p>"
                                     "<p>Databases are digiKam core engines. Take care to use a place hosted by fast "
                                     "hardware (eg. SSD or NVMe) with enough free space especially for thumbnails database.</p>"
                                     "<p>Note: a remote file system such as NFS, cannot be used here. "
                                     "For performance reasons, it is also recommended not to use network storage media.</p>"
                                     "<p></p>"), dbConfigBox);
    d->dbPathLabel->setWordWrap(true);
    d->dbPathEdit  = new DFileSelector(dbConfigBox);
    d->dbPathEdit->setFileDlgMode(QFileDialog::Directory);
    d->dbPathEdit->setFileDlgOptions(QFileDialog::ShowDirsOnly);

    // --------------------------------------------------------

    d->walModeCheck = new QCheckBox(i18n("Enable WAL mode for the databases"), dbConfigBox);
    d->walModeCheck->setToolTip(i18n("The WAL (Write-Ahead Log) mode is significantly "
                                     "faster in most scenarios on supported systems."));

    d->walLabel     = new QLabel(i18n("Write-Ahead Log is a mode to use a roll-forward journal that records transactions "
                                      "that have been committed but not yet applied to the databases. It uses an auxiliary "
                                      "journalized file to host structures for recovery transactions during a crash. The changes "
                                      "are first recorded in the log, before the changes are written to the database. This made "
                                      "database requests atomic and robust in extensive and critical use cases."),
                                 dbConfigBox);
    d->walLabel->setWordWrap(true);

    // --------------------------------------------------------

    d->mysqlCmdBox = new DVBox(dbConfigBox);
    d->mysqlCmdBox->layout()->setContentsMargins(QMargins());

    new DLineWidget(Qt::Horizontal, d->mysqlCmdBox);
    QLabel* const mysqlBinariesLabel  = new QLabel(i18n("<p>Here you can configure locations where MySQL binary tools are located. "
                                                        "digiKam will try to find these binaries automatically if they are "
                                                        "already installed on your computer.</p>"),
                                                   d->mysqlCmdBox);
    mysqlBinariesLabel->setWordWrap(true);

    QGroupBox* const binaryBox        = new QGroupBox(d->mysqlCmdBox);
    QGridLayout* const binaryLayout   = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "MySQL Binaries"));
    d->dbBinariesWidget               = new DBinarySearch(binaryBox);
    d->dbBinariesWidget->header()->setSectionHidden(2, true);

    d->dbBinariesWidget->addBinary(d->mysqlInitBin);
    d->dbBinariesWidget->addBinary(d->mysqlAdminBin);
    d->dbBinariesWidget->addBinary(d->mysqlServBin);

#ifdef Q_OS_LINUX

    d->dbBinariesWidget->addDirectory(QLatin1String("/usr/bin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("/usr/sbin"));

#endif

#ifdef Q_OS_MACOS

    // Std Macports install
    d->dbBinariesWidget->addDirectory(QLatin1String("/opt/local/bin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("/opt/local/sbin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("/opt/local/lib/mariadb/bin"));

    // digiKam Bundle PKG install
    d->dbBinariesWidget->addDirectory(macOSBundlePrefix() + QLatin1String("lib/mariadb/bin"));

#endif

#ifdef Q_OS_WIN

    d->dbBinariesWidget->addDirectory(QLatin1String("C:/Program Files/MariaDB 10.5/bin"));
    d->dbBinariesWidget->addDirectory(QLatin1String("C:/Program Files (x86)/MariaDB 10.5/bin"));

#endif

    d->dbBinariesWidget->allBinariesFound();

    // --------------------------------------------------------

    d->tab                                           = new QTabWidget(this);

    QLabel* const hostNameLabel                      = new QLabel(i18n("Host Name:"));
    d->hostName                                      = new QLineEdit();
    d->hostName->setPlaceholderText(i18n("Set the host computer name"));
    d->hostName->setToolTip(i18n("This is the computer name running MySQL server.\n"
                                 "This can be \"localhost\" for a local server, "
                                 "or the network computer\n"
                                 "name (or IP address) in case of remote computer."));

    QLabel* const connectOptsLabel                   = new QLabel(i18n("<a href=\"https://doc.qt.io/qt-5/"
                                                                       "qsqldatabase.html#setConnectOptions\">Connect options:</a>"));
    connectOptsLabel->setOpenExternalLinks(true);
    d->connectOpts                                   = new QLineEdit();
    d->connectOpts->setPlaceholderText(i18n("Set the database connection options"));
    d->connectOpts->setToolTip(i18n("Set the MySQL server connection options.\n"
                                    "For advanced users only."));

    QLabel* const userNameLabel                      = new QLabel(i18n("User:"));
    d->userName                                      = new QLineEdit();
    d->userName->setPlaceholderText(i18n("Set the database account name"));
    d->userName->setToolTip(i18n("Set the MySQL server account name used\n"
                                 "by digiKam to be connected to the server.\n"
                                 "This account must be available on the remote\n"
                                 "MySQL server when database have been created."));

    QLabel* const passwordLabel                      = new QLabel(i18n("Password:"));
    d->password                                      = new QLineEdit();
    d->password->setToolTip(i18n("Set the MySQL server account password used\n"
                                 "by digiKam to be connected to the server.\n"
                                 "You can left this field empty to use an account set without password."));
    d->password->setEchoMode(QLineEdit::Password);

    DHBox* const phbox                               = new DHBox();
    QLabel* const hostPortLabel                      = new QLabel(i18n("Host Port:"));
    d->hostPort                                      = new QSpinBox(phbox);
    d->hostPort->setToolTip(i18n("Set the host computer port.\nUsually, MySQL server use port number 3306 by default"));
    d->hostPort->setMaximum(65535);
    d->hostPort->setValue(3306);
    QWidget* const space                             = new QWidget(phbox);
    phbox->setStretchFactor(space, 10);
    QPushButton* const checkDBConnectBtn             = new QPushButton(i18n("Check Connection"), phbox);
    checkDBConnectBtn->setToolTip(i18n("Run a basic database connection to see if current MySQL server settings is suitable."));

    // Only accept printable Ascii char for database names.

    QRegularExpression asciiRx(QLatin1String("[\x20-\x7F]+$"));
    QValidator* const asciiValidator = new QRegularExpressionValidator(asciiRx, this);

    QLabel* const dbNameCoreLabel                    = new QLabel(i18n("Core Db Name:"));
    d->dbNameCore                                    = new QLineEdit();
    d->dbNameCore->setPlaceholderText(i18n("Set the core database name"));
    d->dbNameCore->setToolTip(i18n("The core database is lead digiKam container used to store\n"
                                   "albums, items, and searches metadata."));
    d->dbNameCore->setValidator(asciiValidator);

    d->dbThumbsLabel                                 = new QLabel(i18n("Thumbs Db Name:"));
    d->dbNameThumbs                                  = new DFileSelector();
    d->dbNameThumbs->setFileDlgMode(QFileDialog::Directory);
    d->dbNameThumbs->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->dbNameThumbs->lineEdit()->setPlaceholderText(i18n("Set the thumbnails database name or folder"));
    d->dbNameThumbs->setToolTip(i18n("The thumbnails database is used by digiKam to host\n"
                                     "image thumbs with wavelets compression images.\n"
                                     "This one can use quickly a lots of space,\n"
                                     "especially if you have huge collections.\n"
                                     "Choose a local folder to use a SQLite database."));

    QLabel* const dbNameFaceLabel                    = new QLabel(i18n("Face Db Name:"));
    d->dbNameFace                                    = new QLineEdit();
    d->dbNameFace->setPlaceholderText(i18n("Set the face database name"));
    d->dbNameFace->setToolTip(i18n("The face database is used by digiKam to host image histograms\n"
                                   "dedicated to faces recognition process.\n"
                                   "This one can use quickly a lots of space, especially\n"
                                   "if you a lots of image with people faces detected and tagged."));
    d->dbNameFace->setValidator(asciiValidator);

    QLabel* const dbNameSimilarityLabel              = new QLabel(i18n("Similarity Db Name:"));
    d->dbNameSimilarity                              = new QLineEdit();
    d->dbNameSimilarity->setPlaceholderText(i18n("Set the similarity database name"));
    d->dbNameSimilarity->setToolTip(i18n("The similarity database is used by digiKam to host\n"
                                         "image Haar matrix data for the similarity search."));
    d->dbNameSimilarity->setValidator(asciiValidator);

    QPushButton* const defaultValuesBtn              = new QPushButton(i18n("Default Settings"));
    defaultValuesBtn->setToolTip(i18n("Reset database names settings to common default values."));

    d->expertSettings                                = new QGroupBox();
    d->expertSettings->setFlat(true);
    QFormLayout* const expertSettinglayout           = new QFormLayout();
    d->expertSettings->setLayout(expertSettinglayout);

    expertSettinglayout->addRow(hostNameLabel,         d->hostName);
    expertSettinglayout->addRow(userNameLabel,         d->userName);
    expertSettinglayout->addRow(passwordLabel,         d->password);
    expertSettinglayout->addRow(connectOptsLabel,      d->connectOpts);
    expertSettinglayout->addRow(hostPortLabel,         phbox);
    expertSettinglayout->addRow(new DLineWidget(Qt::Horizontal, d->expertSettings));
    expertSettinglayout->addRow(dbNameCoreLabel,       d->dbNameCore);
    expertSettinglayout->addRow(d->dbThumbsLabel,      d->dbNameThumbs);
    expertSettinglayout->addRow(dbNameFaceLabel,       d->dbNameFace);
    expertSettinglayout->addRow(dbNameSimilarityLabel, d->dbNameSimilarity);
    expertSettinglayout->addRow(new QWidget(),         defaultValuesBtn);

    d->tab->addTab(d->expertSettings, i18n("Remote Server Settings"));

    // --------------------------------------------------------

    d->dbNoticeBox           = new QGroupBox(i18n("Database Server Instructions"), this);
    QVBoxLayout* const vlay2 = new QVBoxLayout(d->dbNoticeBox);
    QLabel* const notice     = new QLabel(i18n("<p>digiKam expects that database is already created with a dedicated user account. "
                                               "This user name <i>digikam</i> will require full access to the database.<br/>"
                                               "If your database is not already set up, you can use the following SQL commands "
                                               "(after replacing the <b><i>password</i></b> with the correct one).</p>"),
                                          d->dbNoticeBox);
    notice->setWordWrap(true);

    d->sqlInit                  = new QTextBrowser(d->dbNoticeBox);
    d->sqlInit->setOpenExternalLinks(false);
    d->sqlInit->setOpenLinks(false);
    d->sqlInit->setReadOnly(false);

    QTextBrowser* const notice2 = new QTextBrowser(this);
    notice2->setText(i18n("<p>Note: with a Linux server, a database can be initialized following the commands below:</p>"
                           "<p># su</p>"
                           "<p># systemctl restart mysqld</p>"
                           "<p># mysql -u root</p>"
                           "<p>...</p>"
                           "<p>Enter SQL code to Mysql prompt in order to init digiKam databases with grant privileges (see behind)</p>"
                           "<p>...</p>"
                           "<p>quit</p>"
                           "<p>NOTE: If you have an enormous collection, you should start MySQL server with "
                           "mysql --max_allowed_packet=128M OR in my.ini or ~/.my.cnf, change the settings</p>"));

    notice2->setOpenExternalLinks(false);
    notice2->setOpenLinks(false);
    notice2->setReadOnly(true);

    vlay2->addWidget(notice,     0);
    vlay2->addWidget(d->sqlInit, 10);
    vlay2->addWidget(notice2,    20);
    vlay2->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay2->setSpacing(spacing);

    d->tab->addTab(d->dbNoticeBox, i18n("Requirements"));

    // --------------------------------------------------------

    d->dbDetailsBox          = new QGroupBox(i18n("Database Server Technical Details"), this);
    QVBoxLayout* const vlay3 = new QVBoxLayout(d->dbDetailsBox);
    QLabel* const details    = new QLabel(i18n("<p>Use this configuration view to set all information "
                                               "to be connected to a remote "
                                               "<a href=\"https://en.wikipedia.org/wiki/MySQL\">Mysql database server</a> "
                                               "(or <a href=\"https://en.wikipedia.org/wiki/MariaDB\">MariaDB</a>) "
                                               "through a network. "
                                               "As with Sqlite or MySQL internal server, 3 databases will be stored "
                                               "on the remote server: one for all collections properties, "
                                               "one to store compressed thumbnails, and one to store faces "
                                               "recognition metadata.</p>"
                                               "<p>Unlike Sqlite or MySQL internal server, you can customize the "
                                               "database names to simplify your backups.</p>"
                                               "<p>Databases are digiKam core engines. To prevent performance issues, "
                                               "take a care to use a fast network link between the client and the server "
                                               "computers. It is also recommended to host database files on "
                                               "fast hardware (as <a href=\"https://en.wikipedia.org/wiki/Solid-state_drive\">SSD</a>) "
                                               "with enough free space, especially for thumbnails database, even if data are compressed using wavelets image format <a href=\"https://en.wikipedia.org/wiki/Progressive_Graphics_File\">"
                                               "PGF</a>.</p>"
                                               "<p>The databases must be created previously on the remote server by the administrator. "
                                               "Look in <b>Requirements</b> tab for details.</p>"),
                                          d->dbDetailsBox);
    details->setWordWrap(true);

    vlay3->addWidget(details);
    vlay3->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay3->setSpacing(spacing);

    d->tab->addTab(d->dbDetailsBox, i18n("Documentation"));

    // --------------------------------------------------------

    vlay->addWidget(typeHbox);
    vlay->addWidget(new DLineWidget(Qt::Horizontal));
    vlay->addWidget(d->dbPathLabel);
    vlay->addWidget(d->dbPathEdit);
    vlay->addWidget(d->mysqlCmdBox);
    vlay->addWidget(d->tab);
    vlay->addWidget(d->walModeCheck);
    vlay->addWidget(d->walLabel);
    vlay->addStretch(10);
    vlay->setContentsMargins(spacing, spacing, spacing, spacing);
    vlay->setSpacing(spacing);

    // --------------------------------------------------------

    layout->setContentsMargins(QMargins());
    layout->setSpacing(spacing);
    layout->addWidget(dbConfigBox);

    // --------------------------------------------------------

    connect(d->dbType, SIGNAL(currentIndexChanged(int)),
            this, SLOT(slotHandleDBTypeIndexChanged(int)));

    connect(checkDBConnectBtn, SIGNAL(clicked()),
            this, SLOT(slotCheckMysqlServerConnection()));

    connect(defaultValuesBtn, SIGNAL(clicked()),
            this, SLOT(slotResetMysqlServerDBNames()));

    connect(d->dbNameCore, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameThumbs->lineEdit(), SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameFace, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->dbNameSimilarity, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    connect(d->userName, SIGNAL(textChanged(QString)),
            this, SLOT(slotUpdateSqlInit()));

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

int DatabaseSettingsWidget::databaseType() const
{
    return d->dbType->currentData().toInt();
}

QString DatabaseSettingsWidget::databasePath() const
{
    return d->dbPathEdit->fileDlgPath();
}

void DatabaseSettingsWidget::setDatabasePath(const QString& path)
{
    d->dbPathEdit->setFileDlgPath(path);
}

DbEngineParameters DatabaseSettingsWidget::orgDatabasePrm() const
{
    return d->orgPrms;
}

QString DatabaseSettingsWidget::databaseBackend() const
{
    switch (databaseType())
    {
        case MysqlInternal:
        case MysqlServer:
        {
            return DbEngineParameters::MySQLDatabaseType();
        }

        default: // SQlite
        {
            return DbEngineParameters::SQLiteDatabaseType();
        }
    }
}

void DatabaseSettingsWidget::slotResetMysqlServerDBNames()
{
    d->dbNameCore->setText(QLatin1String("digikam"));
    d->dbNameThumbs->setFileDlgPath(QLatin1String("digikam"));
    d->dbNameFace->setText(QLatin1String("digikam"));
    d->dbNameSimilarity->setText(QLatin1String("digikam"));
}

void DatabaseSettingsWidget::slotHandleDBTypeIndexChanged(int index)
{
    int dbType = d->dbType->itemData(index).toInt();
    setDatabaseInputFields(dbType);
    handleInternalServer(dbType);
    slotUpdateSqlInit();
}

void DatabaseSettingsWidget::setDatabaseInputFields(int index)
{
    switch (index)
    {
        case SQlite:
        {
            d->dbPathLabel->setVisible(true);
            d->dbPathEdit->setVisible(true);
            d->walModeCheck->setVisible(true);
            d->walLabel->setVisible(true);
            d->mysqlCmdBox->setVisible(false);
            d->tab->setVisible(false);

            connect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            break;
        }

        case MysqlInternal:
        {
            d->dbPathLabel->setVisible(true);
            d->dbPathEdit->setVisible(true);
            d->walModeCheck->setVisible(false);
            d->walLabel->setVisible(false);
            d->mysqlCmdBox->setVisible(true);
            d->tab->setVisible(false);

            connect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                    this, SLOT(slotDatabasePathEditedDelayed()));

            break;
        }

        default: // MysqlServer
        {
            d->dbPathLabel->setVisible(false);
            d->dbPathEdit->setVisible(false);
            d->walModeCheck->setVisible(false);
            d->walLabel->setVisible(false);
            d->mysqlCmdBox->setVisible(false);
            d->tab->setVisible(true);

            disconnect(d->dbPathEdit->lineEdit(), SIGNAL(textChanged(QString)),
                       this, SLOT(slotDatabasePathEditedDelayed()));
            break;
        }
    }
}

void DatabaseSettingsWidget::handleInternalServer(int index)
{
    bool internal = (index == MysqlInternal);

    d->hostName->setDisabled(internal);
    d->hostPort->setDisabled(internal);
    d->dbNameCore->setDisabled(internal);
    d->dbNameThumbs->setDisabled(internal);
    d->dbNameFace->setDisabled(internal);
    d->dbNameSimilarity->setDisabled(internal);
    d->userName->setDisabled(internal);
    d->password->setDisabled(internal);
    d->connectOpts->setDisabled(internal);
}

void DatabaseSettingsWidget::slotUpdateSqlInit()
{
    QString sql = QString::fromLatin1("CREATE USER \'%1\'@\'%2\' IDENTIFIED BY \'<b>password</b>\';<br>")
                                      .arg(d->userName->text())
                                      .arg(d->hostName->text());

    sql += QString::fromLatin1("GRANT ALL ON *.* TO \'%1\'@\'%2\' IDENTIFIED BY \'<b>password</b>\';<br>")
                               .arg(d->userName->text())
                               .arg(d->hostName->text());

    sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                               "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                               .arg(d->dbNameCore->text())
                               .arg(d->dbNameCore->text())
                               .arg(d->userName->text())
                               .arg(d->hostName->text());

    if (isNotEqualToThumbName(d->dbNameCore->text()))
    {
        sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                                   "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                                   .arg(d->dbNameThumbs->fileDlgPath())
                                   .arg(d->dbNameThumbs->fileDlgPath())
                                   .arg(d->userName->text())
                                   .arg(d->hostName->text());
    }

    if (isNotEqualToThumbName(d->dbNameFace->text())   &&
        (d->dbNameFace->text() != d->dbNameCore->text()))
    {
        sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                                   "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                                   .arg(d->dbNameFace->text())
                                   .arg(d->dbNameFace->text())
                                   .arg(d->userName->text())
                                   .arg(d->hostName->text());
    }

    if (isNotEqualToThumbName(d->dbNameSimilarity->text())     &&
        (d->dbNameSimilarity->text() != d->dbNameCore->text()) &&
        (d->dbNameSimilarity->text() != d->dbNameFace->text()))
    {
        sql += QString::fromLatin1("CREATE DATABASE `%1`;<br>"
                                   "GRANT ALL PRIVILEGES ON `%2`.* TO \'%3\'@\'%4\';<br>")
                                   .arg(d->dbNameSimilarity->text())
                                   .arg(d->dbNameSimilarity->text())
                                   .arg(d->userName->text())
                                   .arg(d->hostName->text());
    }

    sql += QLatin1String("FLUSH PRIVILEGES;<br>");

    d->sqlInit->setText(sql);

    QFileInfo thumbDB(d->dbNameThumbs->fileDlgPath());

    if (thumbDB.exists() && thumbDB.isDir())
    {
        d->dbThumbsLabel->setText(i18n("Thumbs Db Folder:"));
    }
    else
    {
        d->dbThumbsLabel->setText(i18n("Thumbs Db Name:"));
    }
}

void DatabaseSettingsWidget::slotCheckMysqlServerConnection()
{
    QString error;

    if (checkMysqlServerConnection(error))
    {
        QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "Database Connection Test"),
                                 i18n("Database connection test successful."));
    }
    else
    {
        QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Database Connection Test"),
                              i18n("Database connection test was not successful. <p>Error was: %1</p>",
                                   error));
    }
}

bool DatabaseSettingsWidget::checkMysqlServerConnectionConfig(QString& error)
{
    if (d->hostName->text().isEmpty())
    {
        error = i18n("The server hostname is empty");
        return false;
    }

    if (d->userName->text().isEmpty())
    {
        error = i18n("The server user name is empty");
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::checkMysqlServerDbNamesConfig(QString& error)
{
    if (d->dbNameCore->text().isEmpty())
    {
        error = i18n("The core database name is empty");
        return false;
    }

    if (d->dbNameThumbs->fileDlgPath().isEmpty())
    {
        error = i18n("The thumbnails database name is empty");
        return false;
    }

    if (d->dbNameFace->text().isEmpty())
    {
        error = i18n("The face database name is empty");
        return false;
    }

    if (d->dbNameSimilarity->text().isEmpty())
    {
        error = i18n("The similarity database name is empty");
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::checkMysqlServerConnection(QString& error)
{
    if (!checkMysqlServerConnectionConfig(error))
    {
        return false;
    }

    bool result = false;

    qApp->setOverrideCursor(Qt::WaitCursor);

    QString databaseID(QLatin1String("ConnectionTest"));

    {
        QSqlDatabase testDatabase = QSqlDatabase::addDatabase(databaseBackend(), databaseID);

        DbEngineParameters prm    = getDbEngineParameters();
        qCDebug(DIGIKAM_DATABASE_LOG) << "Testing DB connection (" << databaseID << ") with these settings:";
        qCDebug(DIGIKAM_DATABASE_LOG) << prm;

        testDatabase.setHostName(prm.hostName);
        testDatabase.setPort(prm.port);
        testDatabase.setUserName(prm.userName);
        testDatabase.setPassword(prm.password);
        testDatabase.setConnectOptions(prm.connectOptions);

        result = testDatabase.open();
        error  = testDatabase.lastError().text();
        testDatabase.close();
    }

    QSqlDatabase::removeDatabase(databaseID);

    qApp->restoreOverrideCursor();

    return result;
}

void DatabaseSettingsWidget::setParametersFromSettings(const ApplicationSettings* const settings,
                                                       const bool& migration)
{
    d->orgPrms = settings->getDbEngineParameters();

    if (d->orgPrms.databaseType == DbEngineParameters::SQLiteDatabaseType())
    {
        d->dbPathEdit->setFileDlgPath(d->orgPrms.getCoreDatabaseNameOrDir());
        d->dbType->setCurrentIndex(d->dbTypeMap[SQlite]);
        d->walModeCheck->setChecked(d->orgPrms.walMode);
        slotResetMysqlServerDBNames();

        if (settings->getDatabaseDirSetAtCmd() && !migration)
        {
            d->dbType->setEnabled(false);
            d->dbPathEdit->setEnabled(false);
            d->dbPathLabel->setText(d->dbPathLabel->text() +
                                    i18n("This path was set as a command line"
                                         "option (--database-directory)."));
        }
    }

#ifdef HAVE_MYSQLSUPPORT

#   ifdef HAVE_INTERNALMYSQL

    else if ((d->orgPrms.databaseType == DbEngineParameters::MySQLDatabaseType()) && d->orgPrms.internalServer)
    {
        d->dbPathEdit->setFileDlgPath(d->orgPrms.internalServerPath());
        d->dbType->setCurrentIndex(d->dbTypeMap[MysqlInternal]);
        d->mysqlAdminBin.setup(QFileInfo(d->orgPrms.internalServerMysqlAdminCmd).absoluteFilePath());
        d->mysqlInitBin.setup(QFileInfo(d->orgPrms.internalServerMysqlInitCmd).absoluteFilePath());
        d->mysqlServBin.setup(QFileInfo(d->orgPrms.internalServerMysqlServCmd).absoluteFilePath());
        d->dbBinariesWidget->allBinariesFound();
        d->walModeCheck->setChecked(false);
        slotResetMysqlServerDBNames();
    }

#   endif

    else
    {
        d->dbType->setCurrentIndex(d->dbTypeMap[MysqlServer]);
        d->dbNameCore->setText(d->orgPrms.databaseNameCore);
        d->dbNameThumbs->setFileDlgPath(d->orgPrms.databaseNameThumbnails);
        d->dbNameFace->setText(d->orgPrms.databaseNameFace);
        d->dbNameSimilarity->setText(d->orgPrms.databaseNameSimilarity);
        d->hostName->setText(d->orgPrms.hostName);
        d->hostPort->setValue((d->orgPrms.port == -1) ? 3306 : d->orgPrms.port);
        d->connectOpts->setText(d->orgPrms.connectOptions);
        d->userName->setText(d->orgPrms.userName);
        d->password->setText(d->orgPrms.password);
        d->walModeCheck->setChecked(false);
    }

#endif

    slotHandleDBTypeIndexChanged(d->dbType->currentIndex());
}

DbEngineParameters DatabaseSettingsWidget::getDbEngineParameters() const
{
    DbEngineParameters prm;

    switch (databaseType())
    {
        case SQlite:
        {
            prm         = DbEngineParameters::parametersForSQLiteDefaultFile(databasePath());
            prm.walMode = d->walModeCheck->isChecked();
            break;
        }

        case MysqlInternal:
        {
            prm = DbEngineParameters::defaultParameters(databaseBackend());
            prm.setInternalServerPath(databasePath());
            prm.internalServerMysqlAdminCmd = d->mysqlAdminBin.path();
            prm.internalServerMysqlInitCmd  = d->mysqlInitBin.path();
            prm.internalServerMysqlServCmd  = d->mysqlServBin.path();
            break;
        }

        default: // MysqlServer
        {
            prm.internalServer         = false;
            prm.databaseType           = databaseBackend();
            prm.databaseNameCore       = d->dbNameCore->text();
            prm.databaseNameThumbnails = d->dbNameThumbs->fileDlgPath();
            prm.databaseNameFace       = d->dbNameFace->text();
            prm.databaseNameSimilarity = d->dbNameSimilarity->text();
            prm.connectOptions         = d->connectOpts->text();
            prm.hostName               = d->hostName->text();
            prm.port                   = d->hostPort->value();
            prm.userName               = d->userName->text();
            prm.password               = d->password->text();
            break;
        }
    }

    return prm;
}

void DatabaseSettingsWidget::slotDatabasePathEditedDelayed()
{
    QTimer::singleShot(300, this, SLOT(slotDatabasePathEdited()));
}

void DatabaseSettingsWidget::slotDatabasePathEdited()
{
    QString newPath = databasePath();

#ifndef Q_OS_WIN

    if (!newPath.isEmpty() && !QDir::isAbsolutePath(newPath))
    {
        d->dbPathEdit->setFileDlgPath(QDir::homePath() + QLatin1Char('/') + newPath);
    }

#endif

    d->dbPathEdit->setFileDlgPath(newPath);
}

bool DatabaseSettingsWidget::checkDatabaseSettings()
{
    switch (databaseType())
    {
        case SQlite:
        {
            return checkDatabasePath();
        }

        case MysqlInternal:
        {
            if (!checkDatabasePath())
            {
                return false;
            }

            if (!d->dbBinariesWidget->allBinariesFound())
            {
                return false;
            }

            return true;
        }

        default:  // MysqlServer
        {
            QString error;

            if (!checkMysqlServerDbNamesConfig(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Database Configuration"),
                                      i18n("The database names configuration is not valid. Error is <br/><p>%1</p><br/>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }

            if (!checkMysqlServerConnection(error))
            {
                QMessageBox::critical(qApp->activeWindow(), i18nc("@title:window", "Database Connection Test"),
                                      i18n("Testing database connection has failed with error<br/><p>%1</p><br/>"
                                           "Please check your configuration.",
                                           error));
                return false;
            }
        }
    }

    return true;
}

bool DatabaseSettingsWidget::checkDatabasePath()
{
    QString dbFolder = databasePath();
    qCDebug(DIGIKAM_DATABASE_LOG) << "Database directory is : " << dbFolder;

    if (dbFolder.isEmpty())
    {
        QMessageBox::information(qApp->activeWindow(), qApp->applicationName(),
                                i18n("You must select a folder for digiKam to "
                                     "store information and metadata in a database file."));
        return false;
    }

    QDir targetPath(dbFolder);

    if (!targetPath.exists())
    {
        int rc = QMessageBox::question(qApp->activeWindow(), i18nc("@title:window", "Create Database Folder?"),
                                    i18n("<p>The folder to put your database in does not seem to exist:</p>"
                                         "<p><b>%1</b></p>"
                                         "Would you like digiKam to create it for you?", dbFolder));

        if (rc == QMessageBox::No)
        {
            return false;
        }

        if (!targetPath.mkpath(dbFolder))
        {
            QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "Create Database Folder Failed"),
                                    i18n("<p>digiKam could not create the folder to host your database file.\n"
                                         "Please select a different location.</p>"
                                         "<p><b>%1</b></p>", dbFolder));
            return false;
        }
    }

    QFileInfo path(dbFolder);

#ifdef Q_OS_WIN

    // Work around bug #189168

    QTemporaryFile temp;
    temp.setFileTemplate(path.filePath() + QLatin1String("/XXXXXX"));

    if (!temp.open())

#else

    if (!path.isWritable())

#endif

    {
        QMessageBox::information(qApp->activeWindow(), i18nc("@title:window", "No Database Write Access"),
                                 i18n("<p>You do not seem to have write access "
                                      "for the folder to host the database file.<br/>"
                                      "Please select a different location.</p>"
                                      "<p><b>%1</b></p>", dbFolder));
        return false;
    }

    return true;
}

bool DatabaseSettingsWidget::isNotEqualToThumbName(const QString& name)
{
    QFileInfo thumbDB(d->dbNameThumbs->fileDlgPath());
    bool isDir = (thumbDB.exists() && thumbDB.isDir() && thumbDB.isAbsolute());

    return (!isDir && (d->dbNameThumbs->fileDlgPath() !=  name));
}

} // namespace Digikam
