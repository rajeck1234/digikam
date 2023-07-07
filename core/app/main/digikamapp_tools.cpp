/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Extra tools
 *
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::setupSelectToolsAction()
{
    // Create action model

    ActionItemModel* const actionModel = new ActionItemModel(this);
    actionModel->setMode(ActionItemModel::ToplevelMenuCategory | ActionItemModel::SortCategoriesByInsertionOrder);

    // Builtin actions

    QString mainCategory   = i18nc("@title Main Tools",            "Main Tools");
    actionModel->addAction(d->ieAction,                   mainCategory);
    actionModel->addAction(d->openTagMngrAction,          mainCategory);
    actionModel->addAction(d->bqmAction,                  mainCategory);
    actionModel->addAction(d->maintenanceAction,          mainCategory);
    actionModel->addAction(d->scanNewItemsAction,         mainCategory);
    actionModel->addAction(d->ltAction,                   mainCategory);
    actionModel->addAction(d->advSearchAction,            mainCategory);

    QString postCategory   = i18nc("@title Post Processing Tools", "Post-Processing");

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericTool, this))
    {
        actionModel->addAction(ac, postCategory);
    }

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericMetadata, this))
    {
        actionModel->addAction(ac, postCategory);
    }

    QString exportCategory = i18nc("@title Export Tools",          "Export");

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericExport, this))
    {
        actionModel->addAction(ac, exportCategory);
    }

    QString importCategory = i18nc("@title Import Tools",          "Import");

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericImport, this))
    {
        actionModel->addAction(ac, importCategory);
    }

    // setup categorized view

    DCategorizedSortFilterProxyModel* const filterModel = actionModel->createFilterModel();

    ActionCategorizedView* const selectToolsActionView  = new ActionCategorizedView;
    selectToolsActionView->setIconSize(QSize(22, 22));
    selectToolsActionView->setModel(filterModel);
    selectToolsActionView->setupIconMode();
    selectToolsActionView->adjustGridSize();

    connect(selectToolsActionView, SIGNAL(clicked(QModelIndex)),
            actionModel, SLOT(trigger(QModelIndex)));

    d->view->setToolsIconView(selectToolsActionView);
}

void DigikamApp::slotMaintenance()
{
    QPointer<MaintenanceDlg> dlg = new MaintenanceDlg(this);

    if (dlg->exec() == QDialog::Accepted)
    {
        d->maintenanceAction->setEnabled(false);
        d->scanNewItemsAction->setEnabled(false);

        MaintenanceMngr* const mngr = new MaintenanceMngr(this);

        connect(mngr, SIGNAL(signalComplete()),
                this, SLOT(slotMaintenanceDone()));

        connect(mngr, SIGNAL(signalComplete()),
                mngr, SLOT(deleteLater()));

        mngr->setSettings(dlg->settings());
    }

    delete dlg;
}

void DigikamApp::slotScanNewItems()
{
    d->maintenanceAction->setEnabled(false);
    d->scanNewItemsAction->setEnabled(false);

    NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScanDeferredFiles);

    connect(tool, SIGNAL(signalComplete()),
            this, SLOT(slotMaintenanceDone()));

    connect(tool, SIGNAL(signalCanceled()),
            this, SLOT(slotMaintenanceDone()));

    if (ApplicationSettings::instance()->getDetectFacesInNewImages())
    {
        connect(tool, SIGNAL(signalComplete()),
                this, SLOT(slotDetectFaces()));
    }

    tool->start();
}

void DigikamApp::slotMaintenanceDone()
{
    d->maintenanceAction->setEnabled(true);
    d->scanNewItemsAction->setEnabled(true);

    d->view->refreshView();

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->refreshView();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->refreshView();
    }
}

void DigikamApp::slotDetectFaces()
{
    ItemInfoList newImages(ScanController::instance()->getNewIdsList());

    if (newImages.isEmpty())
    {
        return;
    }

    FaceScanSettings settings;

    settings.accuracy               = ApplicationSettings::instance()->getFaceDetectionAccuracy();
    settings.useYoloV3              = ApplicationSettings::instance()->getFaceDetectionYoloV3();
    settings.task                   = FaceScanSettings::DetectAndRecognize;
    settings.alreadyScannedHandling = FaceScanSettings::Skip;
    settings.infos                  = newImages;

    FacesDetector* const tool       = new FacesDetector(settings);
    tool->start();
}

void DigikamApp::slotDatabaseMigration()
{
    DatabaseMigrationDialog dlg(this);
    (void)dlg.exec();
}

} // namespace Digikam
