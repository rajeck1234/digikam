/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dbmigrationdlg.h"

// QT includes

#include <QApplication>
#include <QGridLayout>
#include <QProgressBar>
#include <QWidget>
#include <QList>
#include <QSqlQuery>
#include <QMap>
#include <QSqlError>
#include <QLabel>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "applicationsettings.h"
#include "coredbaccess.h"
#include "dbsettingswidget.h"
#include "coredbbackend.h"
#include "dbengineparameters.h"
#include "coredbschemaupdater.h"
#include "coredbcopymanager.h"

namespace Digikam
{

class Q_DECL_HIDDEN DatabaseCopyThread::Private
{
public:

    explicit Private()
    {
    }

    DbEngineParameters fromDbEngineParameters;
    DbEngineParameters toDbEngineParameters;
};

DatabaseCopyThread::DatabaseCopyThread(QWidget* const parent)
    : QThread(parent),
      d      (new Private)
{
}

DatabaseCopyThread::~DatabaseCopyThread()
{
    delete d;
}

void DatabaseCopyThread::run()
{
    m_copyManager.copyDatabases(d->fromDbEngineParameters, d->toDbEngineParameters);
}

void DatabaseCopyThread::init(const DbEngineParameters& fromDbEngineParameters,
                              const DbEngineParameters& toDbEngineParameters)
{
    d->fromDbEngineParameters = fromDbEngineParameters;
    d->toDbEngineParameters   = toDbEngineParameters;
}

// ---------------------------------------------------------------------------

class Q_DECL_HIDDEN DatabaseMigrationDialog::Private
{
public:

    explicit Private()
      : fromDatabaseSettingsWidget  (nullptr),
        toDatabaseSettingsWidget    (nullptr),
        migrateButton               (nullptr),
        cancelButton                (nullptr),
        overallStepTitle            (nullptr),
        progressBar                 (nullptr),
        progressBarSmallStep        (nullptr),
        buttons                     (nullptr),
        copyThread                  (nullptr)
    {
    }

    DatabaseSettingsWidget* fromDatabaseSettingsWidget;
    DatabaseSettingsWidget* toDatabaseSettingsWidget;
    QPushButton*            migrateButton;
    QPushButton*            cancelButton;
    QLabel*                 overallStepTitle;
    QProgressBar*           progressBar;
    QProgressBar*           progressBarSmallStep;
    QDialogButtonBox*       buttons;
    DatabaseCopyThread*     copyThread;
};

DatabaseMigrationDialog::DatabaseMigrationDialog(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setupMainArea();
}

DatabaseMigrationDialog::~DatabaseMigrationDialog()
{
    d->copyThread->wait();
    delete d;
}

void DatabaseMigrationDialog::setupMainArea()
{
    d->buttons = new QDialogButtonBox(QDialogButtonBox::Close | QDialogButtonBox::Help, this);
    d->buttons->button(QDialogButtonBox::Close)->setDefault(true);

    d->copyThread                      = new DatabaseCopyThread(this);
    d->fromDatabaseSettingsWidget      = new DatabaseSettingsWidget(this);
    d->toDatabaseSettingsWidget        = new DatabaseSettingsWidget(this);
    d->migrateButton                   = new QPushButton(i18n("Migrate ->"), this);
    d->cancelButton                    = new QPushButton(i18n("Cancel"), this);
    d->cancelButton->setEnabled(false);

    QGroupBox* const progressBox       = new QGroupBox(i18n("Progress Information"), this);
    QGridLayout* const grid            = new QGridLayout(progressBox);

    d->progressBar                     = new QProgressBar(progressBox);
    d->progressBar->setTextVisible(true);
    d->progressBar->setRange(0, 22);
    d->progressBarSmallStep            = new QProgressBar(progressBox);
    d->progressBarSmallStep->setTextVisible(true);

    d->overallStepTitle                = new QLabel(i18n("Step Progress"), progressBox);
    QLabel* const overallSteps         = new QLabel(i18n("Overall Progress"), progressBox);

    grid->addWidget(d->overallStepTitle,     0, 0, 1, 1);
    grid->addWidget(new QLabel(this),        0, 1, 1, 1);
    grid->addWidget(overallSteps,            0, 2, 1, 1);
    grid->addWidget(d->progressBarSmallStep, 1, 0, 1, 1);
    grid->addWidget(new QLabel(this),        1, 1, 1, 1);
    grid->addWidget(d->progressBar,          1, 2, 1, 1);
    grid->setColumnStretch(0, 10);
    grid->setColumnStretch(1, 2);
    grid->setColumnStretch(2, 10);

    QWidget* const mainWidget = new QWidget;
    QGridLayout* const layout = new QGridLayout;
    mainWidget->setLayout(layout);

    layout->addWidget(d->fromDatabaseSettingsWidget, 0, 0, 4, 1);
    layout->addWidget(d->migrateButton,              1, 1);
    layout->addWidget(d->cancelButton,               2, 1);
    layout->addWidget(d->toDatabaseSettingsWidget,   0, 2, 4, 1);
    layout->addWidget(progressBox,                   4, 0, 1, 3);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(mainWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    dataInit();

    // --------------------------------------------------------------------------

    connect(d->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(d->migrateButton, SIGNAL(clicked()),
            this, SLOT(slotPerformCopy()));

    // connect signal handlers for copy d->copyThread

    connect(&(d->copyThread->m_copyManager), SIGNAL(finished(int,QString)),
            this, SLOT(slotHandleFinish(int,QString)));

    connect(&(d->copyThread->m_copyManager), SIGNAL(stepStarted(QString)),
            this, SLOT(slotHandleStepStarted(QString)));

    connect(&(d->copyThread->m_copyManager), SIGNAL(smallStepStarted(int,int)),
            this, SLOT(slotHandleSmallStepStarted(int,int)));

    connect(d->buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            &(d->copyThread->m_copyManager), SLOT(stopProcessing()));

    connect(d->cancelButton, SIGNAL(clicked()),
            &(d->copyThread->m_copyManager), SLOT(stopProcessing()));
}

void DatabaseMigrationDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("setup_application"), QLatin1String("database_settings"), QLatin1String("database_migration"));
}

void DatabaseMigrationDialog::slotPerformCopy()
{
    DbEngineParameters toDBParameters   = d->toDatabaseSettingsWidget->getDbEngineParameters();
    DbEngineParameters fromDBParameters = d->fromDatabaseSettingsWidget->getDbEngineParameters();

    if (fromDBParameters == toDBParameters)
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Database type or location must be different!"));
        return;
    }

    if (fromDBParameters.internalServer && toDBParameters.internalServer)
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Internal server can only be used once!"));
        return;
    }

    DbEngineParameters orgPrms = ApplicationSettings::instance()->getDbEngineParameters();

    if ((fromDBParameters.internalServer || toDBParameters.internalServer) && !orgPrms.internalServer)
    {
        QMessageBox::critical(this, qApp->applicationName(),
                              i18n("Internal server is not used and is not active!"));
        return;
    }

    d->copyThread->init(fromDBParameters, toDBParameters);

    slotLockInputFields();
    d->copyThread->start();
}

void DatabaseMigrationDialog::dataInit()
{
    d->fromDatabaseSettingsWidget->setParametersFromSettings(ApplicationSettings::instance(), true);
    d->toDatabaseSettingsWidget->setParametersFromSettings(ApplicationSettings::instance(),   true);
}

void DatabaseMigrationDialog::slotUnlockInputFields()
{
    d->fromDatabaseSettingsWidget->setEnabled(true);
    d->toDatabaseSettingsWidget->setEnabled(true);
    d->migrateButton->setEnabled(true);
    d->progressBar->setValue(0);
    d->progressBarSmallStep->setValue(0);

    d->cancelButton->setEnabled(false);
}

void DatabaseMigrationDialog::slotLockInputFields()
{
    d->fromDatabaseSettingsWidget->setEnabled(false);
    d->toDatabaseSettingsWidget->setEnabled(false);
    d->migrateButton->setEnabled(false);
    d->cancelButton->setEnabled(true);
}

void DatabaseMigrationDialog::slotHandleFinish(int finishState, const QString& errorMsg)
{
    switch (finishState)
    {
        case CoreDbCopyManager::failed:
            QMessageBox::critical(this, qApp->applicationName(), errorMsg);
            slotUnlockInputFields();
            break;

        case CoreDbCopyManager::success:
            QMessageBox::information(this, qApp->applicationName(), i18n("Database copied successfully."));
            slotUnlockInputFields();
            break;

        case CoreDbCopyManager::canceled:
            QMessageBox::information(this, qApp->applicationName(), i18n("Database conversion canceled."));
            slotUnlockInputFields();
            break;
    }
}

void DatabaseMigrationDialog::slotHandleStepStarted(const QString& stepName)
{
    int progressBarValue = d->progressBar->value();
    d->overallStepTitle->setText(i18n("Step Progress (%1)", stepName));
    d->progressBar->setValue(++progressBarValue);
}

void DatabaseMigrationDialog::slotHandleSmallStepStarted(int currentValue, int maxValue)
{
    d->progressBarSmallStep->setMaximum(maxValue);
    d->progressBarSmallStep->setValue(currentValue);
}

} // namespace Digikam
