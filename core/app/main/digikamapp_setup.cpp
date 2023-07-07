/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation - Internal setup
 *
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"

namespace Digikam
{

void DigikamApp::setupView()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18nc("@info: setup", "Initializing Main View..."));
    }
    else
    {
        qApp->processEvents();
    }

    d->view = new ItemIconView(this, d->modelCollection);
    setCentralWidget(d->view);
    d->view->applySettings();
}

void DigikamApp::setupViewConnections()
{
    connect(d->view, SIGNAL(signalAlbumSelected(Album*)),
            this, SLOT(slotAlbumSelected(Album*)));

    connect(d->view, SIGNAL(signalSelectionChanged(int)),
            this, SLOT(slotSelectionChanged(int)));

    connect(d->view, SIGNAL(signalTrashSelectionChanged(QString)),
            this, SLOT(slotTrashSelectionChanged(QString)));

    connect(d->view, SIGNAL(signalImageSelected(ItemInfoList,ItemInfoList)),
            this, SLOT(slotImageSelected(ItemInfoList,ItemInfoList)));

    connect(d->view, SIGNAL(signalSwitchedToPreview()),
            this, SLOT(slotSwitchedToPreview()));

    connect(d->view, SIGNAL(signalSwitchedToIconView()),
            this, SLOT(slotSwitchedToIconView()));

    connect(d->view, SIGNAL(signalSwitchedToMapView()),
            this, SLOT(slotSwitchedToMapView()));

    connect(d->view, SIGNAL(signalSwitchedToTableView()),
            this, SLOT(slotSwitchedToTableView()));

    connect(d->view, SIGNAL(signalSwitchedToTrashView()),
            this, SLOT(slotSwitchedToTrashView()));
}

void DigikamApp::setupStatusBar()
{
    d->statusLabel = new DAdjustableLabel(statusBar());
    d->statusLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    statusBar()->addWidget(d->statusLabel, 80);

    //------------------------------------------------------------------------------

    d->metadataStatusBar = new MetadataStatusBar(statusBar());
    statusBar()->addWidget(d->metadataStatusBar, 50);

    //------------------------------------------------------------------------------

    d->filterStatusBar = new FilterStatusBar(statusBar());
    statusBar()->addWidget(d->filterStatusBar, 50);
    d->view->connectIconViewFilter(d->filterStatusBar);

    //------------------------------------------------------------------------------

    ProgressView* const view = new ProgressView(statusBar(), this);
    view->hide();

    StatusbarProgressWidget* const littleProgress = new StatusbarProgressWidget(view, statusBar());
    littleProgress->show();
    statusBar()->addPermanentWidget(littleProgress);

    //------------------------------------------------------------------------------

    d->zoomBar = new DZoomBar(statusBar());
    d->zoomBar->setZoomToFitAction(d->zoomFitToWindowAction);
    d->zoomBar->setZoomTo100Action(d->zoomTo100percents);
    d->zoomBar->setZoomPlusAction(d->zoomPlusAction);
    d->zoomBar->setZoomMinusAction(d->zoomMinusAction);
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    statusBar()->addPermanentWidget(d->zoomBar);

    //------------------------------------------------------------------------------

    connect(d->zoomBar, SIGNAL(signalZoomSliderChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->zoomBar, SLOT(slotUpdateTrackerPos()));

    connect(d->zoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->view, SLOT(setZoomFactor(double)));

    connect(d->view, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->view, SIGNAL(signalThumbSizeChanged(int)),
            this, SLOT(slotThumbSizeChanged(int)));
}

void DigikamApp::setupActions()
{
    KActionCollection* const ac = actionCollection();

    d->solidCameraActionGroup   = new QActionGroup(this);
    connect(d->solidCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidCamera(QAction*)));

    d->solidUsmActionGroup      = new QActionGroup(this);
    connect(d->solidUsmActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenSolidUsmDevice(QAction*)));

    d->manualCameraActionGroup  = new QActionGroup(this);
    connect(d->manualCameraActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotOpenManualCamera(QAction*)));

    // -----------------------------------------------------------------

    d->backwardActionMenu = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("go-previous")),
                                                    i18nc("@action: setup", "&Back"), this);
    d->backwardActionMenu->setEnabled(false);
    ac->addAction(QLatin1String("album_back"), d->backwardActionMenu);
    ac->setDefaultShortcut(d->backwardActionMenu, Qt::ALT | Qt::Key_Left);

    connect(d->backwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowBackwardMenu()));

    connect(d->backwardActionMenu, &QAction::triggered,
            this, [this]() { d->view->slotAlbumHistoryBack(1); });

    // -----------------------------------------------------------------

    d->forwardActionMenu = new KToolBarPopupAction(QIcon::fromTheme(QLatin1String("go-next")),
                                                   i18nc("@action: setup", "Forward"), this);
    d->forwardActionMenu->setEnabled(false);
    ac->addAction(QLatin1String("album_forward"), d->forwardActionMenu);
    ac->setDefaultShortcut(d->forwardActionMenu, Qt::ALT | Qt::Key_Right);

    connect(d->forwardActionMenu->menu(), SIGNAL(aboutToShow()),
            this, SLOT(slotAboutToShowForwardMenu()));

    connect(d->forwardActionMenu, &QAction::triggered,
            this, [this]() { d->view->slotAlbumHistoryForward(1); });

    // -----------------------------------------------------------------

    d->refreshAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                                   i18nc("@action: setup", "Refresh"), this);
    d->refreshAction->setWhatsThis(i18nc("@info: setup", "Refresh the current contents."));
    connect(d->refreshAction, SIGNAL(triggered()), d->view, SLOT(slotRefresh()));
    ac->addAction(QLatin1String("view_refresh"), d->refreshAction);
    ac->setDefaultShortcuts(d->refreshAction,  QList<QKeySequence>() << Qt::Key_F5 << (Qt::CTRL | Qt::Key_F5));

    // -----------------------------------------------------------------

    Q_FOREACH (SidebarWidget* const leftWidget, d->view->leftSidebarWidgets())
    {
        QString actionName = QLatin1String("browse_") + leftWidget->objectName()
                                                        .remove(QLatin1Char(' '))
                                                        .remove(QLatin1String("Sidebar"))
                                                        .remove(QLatin1String("FolderView"))
                                                        .remove(QLatin1String("View")).toLower();
        qCDebug(DIGIKAM_GENERAL_LOG) << actionName;

        QAction* const action = new QAction(leftWidget->getIcon(), leftWidget->getCaption(), this);
        ac->addAction(actionName, action);
        ac->setDefaultShortcut(action, QKeySequence(leftWidget->property("Shortcut").toInt()));

        connect(action, &QAction::triggered,
                this, [this, leftWidget]() { d->view->slotLeftSideBarActivate(leftWidget); });
    }

    // -----------------------------------------------------------------

    d->newAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")),
                               i18nc("@action: setup", "&New..."), this);
    d->newAction->setWhatsThis(i18nc("@info: setup", "Creates a new empty Album in the collection."));
    connect(d->newAction, SIGNAL(triggered()), d->view, SLOT(slotNewAlbum()));
    ac->addAction(QLatin1String("album_new"), d->newAction);
    ac->setDefaultShortcut(d->newAction, Qt::CTRL | Qt::Key_N);

    // -----------------------------------------------------------------

    d->moveSelectionToAlbumAction = new QAction(QIcon::fromTheme(QLatin1String("folder-new")),
                                                i18nc("@action: setup", "&Move to Album..."), this);
    d->moveSelectionToAlbumAction->setWhatsThis(i18nc("@info: setup", "Move selected images into an album."));
    connect(d->moveSelectionToAlbumAction, SIGNAL(triggered()), d->view, SLOT(slotMoveSelectionToAlbum()));
    ac->addAction(QLatin1String("move_selection_to_album"), d->moveSelectionToAlbumAction);

    // -----------------------------------------------------------------

    d->copySelectionToAction = new QAction(QIcon::fromTheme(QLatin1String("edit-copy")),
                                           i18nc("@action: setup", "&Copy to..."), this);
    d->copySelectionToAction->setWhatsThis(i18nc("@info: setup", "Copy selected images to a target folder."));
    connect(d->copySelectionToAction, SIGNAL(triggered()), d->view, SLOT(slotCopySelectionTo()));
    ac->addAction(QLatin1String("copy_selection_to"), d->copySelectionToAction);

    // -----------------------------------------------------------------

    d->deleteAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                  i18nc("@action: setup", "Delete Album"), this);
    connect(d->deleteAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteAlbum()));
    ac->addAction(QLatin1String("album_delete"), d->deleteAction);

    // -----------------------------------------------------------------

    d->renameAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                  i18nc("@action: setup", "Rename..."), this);
    connect(d->renameAction, SIGNAL(triggered()), d->view, SLOT(slotRenameAlbum()));
    ac->addAction(QLatin1String("album_rename"), d->renameAction);
    ac->setDefaultShortcut(d->renameAction, Qt::SHIFT | Qt::Key_F2);

    // -----------------------------------------------------------------

    d->propsEditAction = new QAction(QIcon::fromTheme(QLatin1String("configure")),
                                     i18nc("@action: setup", "Properties"), this);
    d->propsEditAction->setWhatsThis(i18nc("@info: setup", "Edit album properties and collection information."));
    connect(d->propsEditAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumPropsEdit()));
    ac->addAction(QLatin1String("album_propsEdit"), d->propsEditAction);
    ac->setDefaultShortcut(d->propsEditAction, Qt::ALT | Qt::Key_Return);

    // -----------------------------------------------------------------

    d->writeAlbumMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                              i18nc("@action: setup", "Write Metadata to Files"), this);
    d->writeAlbumMetadataAction->setWhatsThis(i18nc("@info: setup", "Updates metadata of files in the current "
                                                    "album with the contents of digiKam database "
                                                    "(file metadata will be overwritten with data from "
                                                    "the database)."));
    connect(d->writeAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumWriteMetadata()));
    ac->addAction(QLatin1String("album_write_metadata"), d->writeAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->readAlbumMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("edit-redo")),
                                             i18nc("@action: setup", "Reread Metadata From Files"), this);
    d->readAlbumMetadataAction->setWhatsThis(i18nc("@info: setup", "Updates the digiKam database from the metadata "
                                                   "of the files in the current album "
                                                   "(information in the database will be overwritten with data from "
                                                   "the files' metadata)."));
    connect(d->readAlbumMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumReadMetadata()));
    ac->addAction(QLatin1String("album_read_metadata"), d->readAlbumMetadataAction);

    // -----------------------------------------------------------------

    d->openInFileManagerAction = new QAction(QIcon::fromTheme(QLatin1String("folder-open")),
                                             i18nc("@action: setup", "Open in File Manager"), this);
    connect(d->openInFileManagerAction, SIGNAL(triggered()), d->view, SLOT(slotAlbumOpenInFileManager()));
    ac->addAction(QLatin1String("album_openinfilemanager"), d->openInFileManagerAction);

    // -----------------------------------------------------------

    d->openTagMngrAction = new QAction(QIcon::fromTheme(QLatin1String("tag")),
                                       i18nc("@action: setup", "Tag Manager"), this);
    connect(d->openTagMngrAction, SIGNAL(triggered()), d->view, SLOT(slotOpenTagsManager()));
    ac->addAction(QLatin1String("open_tag_mngr"), d->openTagMngrAction);

    // -----------------------------------------------------------

    d->newTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")),
                                  i18nc("@action: new tag", "N&ew..."), this);
    connect(d->newTagAction, SIGNAL(triggered()), d->view, SLOT(slotNewTag()));
    ac->addAction(QLatin1String("tag_new"), d->newTagAction);

    // -----------------------------------------------------------

    d->editTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-properties")),
                                   i18nc("@action: setup", "Properties"), this);
    connect(d->editTagAction, SIGNAL(triggered()), d->view, SLOT(slotEditTag()));
    ac->addAction(QLatin1String("tag_edit"), d->editTagAction);

    // -----------------------------------------------------------

    d->deleteTagAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                     i18nc("@action: setup", "Delete"), this);
    connect(d->deleteTagAction, SIGNAL(triggered()), d->view, SLOT(slotDeleteTag()));
    ac->addAction(QLatin1String("tag_delete"), d->deleteTagAction);

    // -----------------------------------------------------------

    d->assignTagAction = new QAction(QIcon::fromTheme(QLatin1String("tag-new")),
                                     i18nc("@action: setup", "Assign Tag"), this);
    connect(d->assignTagAction, SIGNAL(triggered()), d->view, SLOT(slotAssignTag()));
    ac->addAction(QLatin1String("tag_assign"), d->assignTagAction);
    ac->setDefaultShortcut(d->assignTagAction, Qt::Key_T);

    // -----------------------------------------------------------

    d->imageViewSelectionAction = new KSelectAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                                    i18nc("@action: setup", "Views"), this);
    ac->addAction(QLatin1String("view_selection"), d->imageViewSelectionAction);

    d->imageIconViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-icons")),
                                         i18nc("@action: Go to thumbnails (icon) view", "Thumbnails"), this);
    d->imageIconViewAction->setCheckable(true);
    ac->addAction(QLatin1String("icon_view"), d->imageIconViewAction);
    connect(d->imageIconViewAction, SIGNAL(triggered()), d->view, SLOT(slotIconView()));
    d->imageViewSelectionAction->addAction(d->imageIconViewAction);

    d->imagePreviewAction = new QAction(QIcon::fromTheme(QLatin1String("view-preview")),
                                        i18nc("@action: View the selected image", "Preview"), this);
    d->imagePreviewAction->setCheckable(true);
    ac->addAction(QLatin1String("image_view"), d->imagePreviewAction);
    ac->setDefaultShortcut(d->imagePreviewAction, Qt::Key_F3);
    connect(d->imagePreviewAction, SIGNAL(triggered()), d->view, SLOT(slotImagePreview()));
    d->imageViewSelectionAction->addAction(d->imagePreviewAction);

#ifdef HAVE_MARBLE

    d->imageMapViewAction = new QAction(QIcon::fromTheme(QLatin1String("globe")),
                                        i18nc("@action: Switch to map view", "Map"), this);
    d->imageMapViewAction->setCheckable(true);
    ac->addAction(QLatin1String("map_view"), d->imageMapViewAction);
    connect(d->imageMapViewAction, SIGNAL(triggered()), d->view, SLOT(slotMapWidgetView()));
    d->imageViewSelectionAction->addAction(d->imageMapViewAction);

#endif // HAVE_MARBLE

    d->imageTableViewAction = new QAction(QIcon::fromTheme(QLatin1String("view-list-details")),
                                          i18nc("@action: Switch to table view", "Table"), this);
    d->imageTableViewAction->setCheckable(true);
    ac->addAction(QLatin1String("table_view"), d->imageTableViewAction);
    connect(d->imageTableViewAction, SIGNAL(triggered()), d->view, SLOT(slotTableView()));
    d->imageViewSelectionAction->addAction(d->imageTableViewAction);

    // -----------------------------------------------------------

    d->imageViewAction = new QAction(QIcon::fromTheme(QLatin1String("quickopen-file")),
                                     i18nc("@action: setup", "Open..."), this);
    d->imageViewAction->setWhatsThis(i18nc("@info: setup", "Open the selected item."));
    connect(d->imageViewAction, SIGNAL(triggered()), d->view, SLOT(slotImageEdit()));
    ac->addAction(QLatin1String("image_edit"), d->imageViewAction);
    ac->setDefaultShortcut(d->imageViewAction, Qt::Key_F4);

    d->openWithAction = new QAction(QIcon::fromTheme(QLatin1String("preferences-desktop-filetype-association")),
                                    i18nc("@action: setup", "Open With Default Application"), this);
    d->openWithAction->setWhatsThis(i18nc("@info: setup", "Open the selected item with default assigned application."));
    connect(d->openWithAction, SIGNAL(triggered()), d->view, SLOT(slotFileWithDefaultApplication()));
    ac->addAction(QLatin1String("open_with_default_application"), d->openWithAction);
    ac->setDefaultShortcut(d->openWithAction, Qt::CTRL | Qt::Key_F4);

    d->ieAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                              i18nc("@action: setup", "Image Editor"), this);
    d->ieAction->setWhatsThis(i18nc("@action: setup", "Open the image editor."));
    connect(d->ieAction, SIGNAL(triggered()), d->view, SLOT(slotEditor()));
    ac->addAction(QLatin1String("imageeditor"), d->ieAction);

    // -----------------------------------------------------------

    d->ltAction = new QAction(QIcon::fromTheme(QLatin1String("lighttable")),
                              i18nc("@action: setup", "Light Table"), this);
    connect(d->ltAction, SIGNAL(triggered()), d->view, SLOT(slotLightTable()));
    ac->addAction(QLatin1String("light_table"), d->ltAction);
    ac->setDefaultShortcut(d->ltAction, Qt::SHIFT | Qt::Key_L);

    d->imageLightTableAction = new QAction(QIcon::fromTheme(QLatin1String("lighttable")),
                                           i18nc("@action: setup", "Place onto Light Table"), this);
    d->imageLightTableAction->setWhatsThis(i18nc("@info: setup", "Place the selected items on the light table thumbbar."));
    connect(d->imageLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageLightTable()));
    ac->addAction(QLatin1String("image_lighttable"), d->imageLightTableAction);
    ac->setDefaultShortcut(d->imageLightTableAction, Qt::CTRL | Qt::Key_L);

    d->imageAddLightTableAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")),
                                              i18nc("@action: setup", "Add to Light Table"), this);
    d->imageAddLightTableAction->setWhatsThis(i18nc("@info: setup", "Add selected items to the light table thumbbar."));
    connect(d->imageAddLightTableAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToLightTable()));
    ac->addAction(QLatin1String("image_add_to_lighttable"), d->imageAddLightTableAction);
    ac->setDefaultShortcut(d->imageAddLightTableAction, Qt::CTRL | Qt::SHIFT | Qt::Key_L);

    // -----------------------------------------------------------

    d->bqmAction = new QAction(QIcon::fromTheme(QLatin1String("run-build")),
                               i18nc("@action: setup", "Batch Queue Manager"), this);
    connect(d->bqmAction, SIGNAL(triggered()), d->view, SLOT(slotQueueMgr()));
    ac->addAction(QLatin1String("queue_manager"), d->bqmAction);
    ac->setDefaultShortcut(d->bqmAction, Qt::SHIFT | Qt::Key_B);

    d->imageAddCurrentQueueAction = new QAction(QIcon::fromTheme(QLatin1String("go-up")),
                                                i18nc("@action: setup", "Add to Current Queue"), this);
    d->imageAddCurrentQueueAction->setWhatsThis(i18nc("@info: setup", "Add selected items to current queue from batch manager."));
    connect(d->imageAddCurrentQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToCurrentQueue()));
    ac->addAction(QLatin1String("image_add_to_current_queue"), d->imageAddCurrentQueueAction);
    ac->setDefaultShortcut(d->imageAddCurrentQueueAction, Qt::CTRL | Qt::Key_B);

    d->imageAddNewQueueAction = new QAction(QIcon::fromTheme(QLatin1String("list-add")),
                                            i18nc("@action: setup", "Add to New Queue"), this);
    d->imageAddNewQueueAction->setWhatsThis(i18nc("@info: setup", "Add selected items to a new queue from batch manager."));
    connect(d->imageAddNewQueueAction, SIGNAL(triggered()), d->view, SLOT(slotImageAddToNewQueue()));
    ac->addAction(QLatin1String("image_add_to_new_queue"), d->imageAddNewQueueAction);
    ac->setDefaultShortcut(d->imageAddNewQueueAction, Qt::CTRL | Qt::SHIFT | Qt::Key_B);

    // -----------------------------------------------------------------

    d->quickImportMenu->setTitle(i18nc("@action: Import photos from camera", "Import"));
    d->quickImportMenu->setIcon(QIcon::fromTheme(QLatin1String("camera-photo")));
    ac->addAction(QLatin1String("import_auto"), d->quickImportMenu->menuAction());

    // -----------------------------------------------------------------

    d->imageWriteMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                              i18nc("@action: setup", "Write Metadata to Selected Files"), this);
    d->imageWriteMetadataAction->setWhatsThis(i18nc("@info: setup", "Updates metadata of files in the current "
                                                    "album with the contents of digiKam database "
                                                    "(file metadata will be overwritten with data from "
                                                    "the database)."));
    connect(d->imageWriteMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageWriteMetadata()));
    ac->addAction(QLatin1String("image_write_metadata"), d->imageWriteMetadataAction);

    // -----------------------------------------------------------------

    d->imageReadMetadataAction = new QAction(QIcon::fromTheme(QLatin1String("edit-redo")),
                                             i18nc("@action: setup", "Reread Metadata From Selected Files"), this);
    d->imageReadMetadataAction->setWhatsThis(i18nc("@info: setup", "Updates the digiKam database from the metadata "
                                                   "of the files in the current album "
                                                   "(information in the database will be overwritten with data from "
                                                   "the files' metadata)."));
    connect(d->imageReadMetadataAction, SIGNAL(triggered()), d->view, SLOT(slotImageReadMetadata()));
    ac->addAction(QLatin1String("image_read_metadata"), d->imageReadMetadataAction);

    // -----------------------------------------------------------

    d->imageScanForFacesAction = new QAction(QIcon::fromTheme(QLatin1String("list-add-user")),
                                             i18nc("@action: setup", "Scan for Faces"), this);
    connect(d->imageScanForFacesAction, SIGNAL(triggered()), d->view, SLOT(slotImageScanForFaces()));
    ac->addAction(QLatin1String("image_scan_for_faces"), d->imageScanForFacesAction);

    d->imageRecognizeFacesAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find-user")),
                                               i18nc("@action: setup", "Recognize Faces"), this);
    connect(d->imageRecognizeFacesAction, SIGNAL(triggered()), d->view, SLOT(slotImageRecognizeFaces()));
    ac->addAction(QLatin1String("image_recognize_faces"), d->imageRecognizeFacesAction);

    // -----------------------------------------------------------

    d->imageFindSimilarAction = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")),
                                            i18nc("@action: setup", "Find Similar..."), this);
    d->imageFindSimilarAction->setWhatsThis(i18nc("@info: setup", "Find similar images using selected item as reference."));
    connect(d->imageFindSimilarAction, SIGNAL(triggered()), d->view, SLOT(slotImageFindSimilar()));
    ac->addAction(QLatin1String("image_find_similar"), d->imageFindSimilarAction);

    // -----------------------------------------------------------

    d->imageRenameAction = new QAction(QIcon::fromTheme(QLatin1String("document-edit")),
                                       i18nc("@action: setup", "Rename..."), this);
    d->imageRenameAction->setWhatsThis(i18nc("@info: setup", "Change the filename of the currently selected item."));
    connect(d->imageRenameAction, SIGNAL(triggered()), d->view, SLOT(slotImageRename()));
    ac->addAction(QLatin1String("image_rename"), d->imageRenameAction);
    ac->setDefaultShortcut(d->imageRenameAction, Qt::Key_F2);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to move to trash

    d->imageDeleteAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")),
                                       i18nc("@action: Non-pluralized", "Move to Trash"), this);
    connect(d->imageDeleteAction, SIGNAL(triggered()), d->view, SLOT(slotImageDelete()));
    ac->addAction(QLatin1String("image_delete"), d->imageDeleteAction);
    ac->setDefaultShortcut(d->imageDeleteAction, Qt::Key_Delete);

    // -----------------------------------------------------------

    // Pop up dialog to ask user whether to permanently delete
    // FIXME: This action is never used?? How can someone delete a album directly, without moving it to the trash first?
    //        This is especially important when deleting from a different partition or from a net source.
    //        Also note that we use the wrong icon for the default album delete action, which should have a trashcan icon instead
    //        of a red cross, it confuses users.

    d->imageDeletePermanentlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                  i18nc("@action: setup", "Delete Permanently"), this);
    connect(d->imageDeletePermanentlyAction, SIGNAL(triggered()), d->view, SLOT(slotImageDeletePermanently()));
    ac->addAction(QLatin1String("image_delete_permanently"), d->imageDeletePermanentlyAction);
    ac->setDefaultShortcut(d->imageDeletePermanentlyAction, Qt::SHIFT | Qt::Key_Delete);

    // -----------------------------------------------------------

    // These two actions are hidden, no menu entry, no toolbar entry, no shortcut.
    // Power users may add them.

    d->imageDeletePermanentlyDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("edit-delete")),
                                                          i18nc("@action: setup", "Delete permanently without confirmation"), this);
    connect(d->imageDeletePermanentlyDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageDeletePermanentlyDirectly()));
    ac->addAction(QLatin1String("image_delete_permanently_directly"), d->imageDeletePermanentlyDirectlyAction);

    // -----------------------------------------------------------

    d->imageTrashDirectlyAction = new QAction(QIcon::fromTheme(QLatin1String("user-trash")),
                                              i18nc("@action: setup", "Move to trash without confirmation"), this);
    connect(d->imageTrashDirectlyAction, SIGNAL(triggered()),
            d->view, SLOT(slotImageTrashDirectly()));
    ac->addAction(QLatin1String("image_trash_directly"), d->imageTrashDirectlyAction);

    // -----------------------------------------------------------------

    d->albumSortAction = new KSelectAction(i18nc("@action: setup", "&Sort Albums"), this);
    d->albumSortAction->setWhatsThis(i18nc("@info: setup", "Sort Albums in tree-view."));

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))

    connect(d->albumSortAction, SIGNAL(indexTriggered(int)),
            d->view, SLOT(slotSortAlbums(int)));

#else

    connect(d->albumSortAction, SIGNAL(triggered(int)),
            d->view, SLOT(slotSortAlbums(int)));

#endif

    ac->addAction(QLatin1String("album_sort"), d->albumSortAction);

    // Use same list order as in applicationsettings enum

    QStringList sortActionList;
    sortActionList.append(i18nc("@action: setup", "By Folder"));
    sortActionList.append(i18nc("@action: setup", "By Category"));
    sortActionList.append(i18nc("@action: setup", "By Date"));
    d->albumSortAction->setItems(sortActionList);

    // -----------------------------------------------------------

    d->recurseAlbumsAction = new QAction(i18nc("@action: setup", "Include Album Sub-Tree"), this);
    d->recurseAlbumsAction->setCheckable(true);
    d->recurseAlbumsAction->setWhatsThis(i18nc("@info: setup", "Activate this option to show all sub-albums below "
                                               "the current album."));
    connect(d->recurseAlbumsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseAlbums(bool)));
    ac->addAction(QLatin1String("albums_recursive"), d->recurseAlbumsAction);

    d->recurseTagsAction   = new QAction(i18nc("@action: setup", "Include Tag Sub-Tree"), this);
    d->recurseTagsAction->setCheckable(true);
    d->recurseTagsAction->setWhatsThis(i18nc("@info: setup", "Activate this option to show all images marked by the given tag "
                                             "and all its sub-tags."));
    connect(d->recurseTagsAction, SIGNAL(toggled(bool)), this, SLOT(slotRecurseTags(bool)));
    ac->addAction(QLatin1String("tags_recursive"), d->recurseTagsAction);

    d->allGroupsOpenAction = new QAction(i18nc("@action: setup", "Show All Grouped Items As Open"), this);
    d->allGroupsOpenAction->setCheckable(true);
    d->allGroupsOpenAction->setWhatsThis(i18nc("@info: setup", "Activate this option to show all grouped items as open."));
    connect(d->allGroupsOpenAction, SIGNAL(toggled(bool)), this, SLOT(slotAllGroupsOpen(bool)));
    ac->addAction(QLatin1String("all_groups_open"), d->allGroupsOpenAction);

    // -----------------------------------------------------------

    d->imageSortAction     = new KSelectAction(i18nc("@action: setup", "&Sort Items"), this);
    d->imageSortAction->setWhatsThis(i18nc("@info: setup", "The value by which the images in one album are sorted in the thumbnail view"));
    ac->addAction(QLatin1String("image_sort"), d->imageSortAction);

    // map to ItemSortSettings enum

    QAction* const sortByNameAction               = d->imageSortAction->addAction(i18nc("@action: setup", "By Name"));
    QAction* const sortByPathAction               = d->imageSortAction->addAction(i18nc("@action: setup", "By Path"));
    QAction* const sortByDateAction               = d->imageSortAction->addAction(i18nc("@action: setup", "By Creation Date"));
    QAction* const sortByModDateAction            = d->imageSortAction->addAction(i18nc("@action: setup", "By Modification Date"));
    QAction* const sortByFileSizeAction           = d->imageSortAction->addAction(i18nc("@action: setup", "By File Size"));
    QAction* const sortByRatingAction             = d->imageSortAction->addAction(i18nc("@action: setup", "By Rating"));
    QAction* const sortByImageSizeAction          = d->imageSortAction->addAction(i18nc("@action: setup", "By Image Size"));
    QAction* const sortByAspectRatioAction        = d->imageSortAction->addAction(i18nc("@action: setup", "By Aspect Ratio"));
    QAction* const sortByFacesAction              = d->imageSortAction->addAction(i18nc("@action: setup", "By Face type"));
    QAction* const sortBySimilarityAction         = d->imageSortAction->addAction(i18nc("@action: setup", "By Similarity"));
    QAction* const sortByManualOrderAndNameAction = d->imageSortAction->addAction(i18nc("@action: setup", "By Manual and Name"));
    QAction* const sortByManualOrderAndDateAction = d->imageSortAction->addAction(i18nc("@action: setup", "By Manual and Date"));

    // activate the sort by similarity if the fuzzy search sidebar is active. Deactivate at start.

    sortBySimilarityAction->setEnabled(false);
    connect(d->view, SIGNAL(signalFuzzySidebarActive(bool)), sortBySimilarityAction, SLOT(setEnabled(bool)));

    connect(sortByNameAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByFileName); });

    connect(sortByPathAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByFilePath); });

    connect(sortByDateAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByCreationDate); });

    connect(sortByModDateAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByModificationDate); });

    connect(sortByFileSizeAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByFileSize); });

    connect(sortByRatingAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByRating); });

    connect(sortByImageSizeAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByImageSize); });

    connect(sortByAspectRatioAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByAspectRatio); });

    connect(sortByFacesAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByFaces); });

    connect(sortBySimilarityAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortBySimilarity); });

    connect(sortByManualOrderAndNameAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByManualOrderAndName); });

    connect(sortByManualOrderAndDateAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImages((int)ItemSortSettings::SortByManualOrderAndDate); });

    // -----------------------------------------------------------

    d->imageSortOrderAction             = new KSelectAction(i18nc("@action: setup", "Item Sort &Order"), this);
    d->imageSortOrderAction->setWhatsThis(i18nc("@info: setup", "Defines whether images are sorted in ascending or descending manner."));
    ac->addAction(QLatin1String("image_sort_order"), d->imageSortOrderAction);

    QAction* const sortAscendingAction  = d->imageSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),
                                                                             i18nc("@action: setup", "Ascending"));
    QAction* const sortDescendingAction = d->imageSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")),
                                                                             i18nc("@action: setup", "Descending"));

    connect(sortAscendingAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesOrder((int)ItemSortSettings::AscendingOrder); });

    connect(sortDescendingAction, &QAction::triggered,
            this, [this]() { d->view->slotSortImagesOrder((int)ItemSortSettings::DescendingOrder); });

    // -----------------------------------------------------------

    d->imageSeparationAction              = new KSelectAction(i18nc("@action: setup", "Separate Items"), this);
    d->imageSeparationAction->setWhatsThis(i18nc("@info: setup", "The categories in which the images in the thumbnail view are displayed"));
    ac->addAction(QLatin1String("image_separation"), d->imageSeparationAction);

    // map to ItemSortSettings enum

    QAction* const noCategoriesAction     = d->imageSeparationAction->addAction(i18nc("@action: setup", "Flat List"));
    QAction* const separateByAlbumAction  = d->imageSeparationAction->addAction(i18nc("@action: setup", "By Album"));
    QAction* const separateByFormatAction = d->imageSeparationAction->addAction(i18nc("@action: setup", "By Format"));
    QAction* const separateByMonthAction  = d->imageSeparationAction->addAction(i18nc("@action: setup", "By Month"));
    QAction* const separateByFacesAction  = d->imageSeparationAction->addAction(i18nc("@action: setup", "By Faces"));

    connect(noCategoriesAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)ItemSortSettings::OneCategory); });

    connect(separateByAlbumAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)ItemSortSettings::CategoryByAlbum); });

    connect(separateByFormatAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)ItemSortSettings::CategoryByFormat); });

    connect(separateByMonthAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)ItemSortSettings::CategoryByMonth); });

    connect(separateByFacesAction, &QAction::triggered,
            this, [this]() { d->view->slotSeparateImages((int)ItemSortSettings::CategoryByFaces); });

    connect(d->view, &ItemIconView::signalSeparationModeChanged,
            this, [this](int category)
        {
            d->imageSeparationAction->blockSignals(true);
            d->imageSeparationAction->setCurrentItem(category-1);
            d->imageSeparationAction->blockSignals(false);
        }
    );

    // -----------------------------------------------------------------

    d->imageSeparationSortOrderAction        = new KSelectAction(i18nc("@action: setup", "Item Separation Order"), this);
    d->imageSeparationSortOrderAction->setWhatsThis(i18nc("@info: setup", "The sort order of the groups of separated items"));
    ac->addAction(QLatin1String("image_separation_sort_order"), d->imageSeparationSortOrderAction);

    QAction* const sortSeparationsAscending  = d->imageSeparationSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-ascending")),
                                                                                            i18nc("@action: setup", "Ascending"));
    QAction* const sortSeparationsDescending = d->imageSeparationSortOrderAction->addAction(QIcon::fromTheme(QLatin1String("view-sort-descending")),
                                                                                            i18nc("@action: setup", "Descending"));

    connect(sortSeparationsAscending, &QAction::triggered,
            this, [this]() { d->view->slotImageSeparationSortOrder((int)ItemSortSettings::AscendingOrder); });

    connect(sortSeparationsDescending, &QAction::triggered,
            this, [this]() { d->view->slotImageSeparationSortOrder((int)ItemSortSettings::DescendingOrder); });

    // -----------------------------------------------------------------

    setupImageTransformActions();
    setupExifOrientationActions();

    // -----------------------------------------------------------------

    d->selectAllAction = new QAction(i18nc("@action: setup", "Select All"), this);
    connect(d->selectAllAction, SIGNAL(triggered()), d->view, SLOT(slotSelectAll()));
    ac->addAction(QLatin1String("selectAll"), d->selectAllAction);
    ac->setDefaultShortcut(d->selectAllAction, Qt::CTRL | Qt::Key_A);

    // -----------------------------------------------------------------

    d->selectNoneAction = new QAction(i18nc("@action: setup", "Select None"), this);
    connect(d->selectNoneAction, SIGNAL(triggered()), d->view, SLOT(slotSelectNone()));
    ac->addAction(QLatin1String("selectNone"), d->selectNoneAction);
    ac->setDefaultShortcut(d->selectNoneAction, Qt::CTRL | Qt::SHIFT | Qt::Key_A);

    // -----------------------------------------------------------------

    d->selectInvertAction = new QAction(i18nc("@action: setup", "Invert Selection"), this);
    connect(d->selectInvertAction, SIGNAL(triggered()), d->view, SLOT(slotSelectInvert()));
    ac->addAction(QLatin1String("selectInvert"), d->selectInvertAction);
    ac->setDefaultShortcut(d->selectInvertAction, Qt::CTRL | Qt::Key_I);

    // -----------------------------------------------------------

    d->showBarAction = new QAction(QIcon::fromTheme(QLatin1String("view-choose")),
                                   i18nc("@action: setup", "Show Thumbbar"), this);
    d->showBarAction->setCheckable(true);
    connect(d->showBarAction, SIGNAL(triggered()), this, SLOT(slotToggleShowBar()));
    ac->addAction(QLatin1String("showthumbs"), d->showBarAction);
    ac->setDefaultShortcut(d->showBarAction, Qt::CTRL | Qt::Key_T);

    // Provides a menu entry that allows showing/hiding the toolbar(s)

    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar

    createStandardStatusBarAction();

    // Standard 'Configure' menu actions

    createSettingsActions();

    // -----------------------------------------------------------

    d->zoomPlusAction = buildStdAction(StdZoomInAction, d->view, SLOT(slotZoomIn()), this);
    ac->addAction(QLatin1String("album_zoomin"), d->zoomPlusAction);

    // -----------------------------------------------------------

    d->zoomMinusAction = buildStdAction(StdZoomOutAction, d->view, SLOT(slotZoomOut()), this);
    ac->addAction(QLatin1String("album_zoomout"), d->zoomMinusAction);

    // -----------------------------------------------------------

    d->zoomTo100percents = new QAction(QIcon::fromTheme(QLatin1String("zoom-original")),
                                       i18nc("@action: setup", "Zoom to 100%"), this);
    connect(d->zoomTo100percents, SIGNAL(triggered()), d->view, SLOT(slotZoomTo100Percents()));
    ac->addAction(QLatin1String("album_zoomto100percents"), d->zoomTo100percents);
    ac->setDefaultShortcut(d->zoomTo100percents, Qt::CTRL | Qt::Key_Period);

    // -----------------------------------------------------------

    d->zoomFitToWindowAction = new QAction(QIcon::fromTheme(QLatin1String("zoom-fit-best")),
                                           i18nc("@action: setup", "Fit to &Window"), this);
    connect(d->zoomFitToWindowAction, SIGNAL(triggered()), d->view, SLOT(slotFitToWindow()));
    ac->addAction(QLatin1String("album_zoomfit2window"), d->zoomFitToWindowAction);
    ac->setDefaultShortcut(d->zoomFitToWindowAction, Qt::CTRL | Qt::ALT | Qt::Key_E);

    // -----------------------------------------------------------

    createFullScreenAction(QLatin1String("full_screen"));
    createSidebarActions();

    // -----------------------------------------------------------

    d->viewCMViewAction = new QAction(QIcon::fromTheme(QLatin1String("video-display")),
                                      i18nc("@action: setup", "Color-Managed View"), this);
    d->viewCMViewAction->setCheckable(true);
    connect(d->viewCMViewAction, SIGNAL(triggered()), this, SLOT(slotToggleColorManagedView()));
    ac->addAction(QLatin1String("color_managed_view"), d->viewCMViewAction);
    ac->setDefaultShortcut(d->viewCMViewAction, Qt::Key_F12);

    // -----------------------------------------------------------

    d->quitAction = buildStdAction(StdQuitAction, this, SLOT(slotExit()), this);
    ac->addAction(QLatin1String("app_exit"), d->quitAction);

    // -----------------------------------------------------------

    createHelpActions(QLatin1String("main_window"));

    // -----------------------------------------------------------

    QAction* const findAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")),
                                            i18nc("@action: setup", "Search..."), this);
    connect(findAction, SIGNAL(triggered()), d->view, SLOT(slotNewKeywordSearch()));
    ac->addAction(QLatin1String("search_quick"), findAction);
    ac->setDefaultShortcut(findAction, Qt::CTRL | Qt::Key_F);

    // -----------------------------------------------------------

    d->advSearchAction = new QAction(QIcon::fromTheme(QLatin1String("edit-find")),
                                     i18nc("@action: setup", "Advanced Search..."), this);
    connect(d->advSearchAction, SIGNAL(triggered()), d->view, SLOT(slotNewAdvancedSearch()));
    ac->addAction(QLatin1String("search_advanced"), d->advSearchAction);
    ac->setDefaultShortcut(d->advSearchAction, Qt::CTRL | Qt::ALT | Qt::Key_F);

    // -----------------------------------------------------------

    QAction* const duplicatesAction = new QAction(QIcon::fromTheme(QLatin1String("tools-wizard")),
                                                  i18nc("@action: setup", "Find Duplicates..."), this);
    connect(duplicatesAction, SIGNAL(triggered()), d->view, SLOT(slotNewDuplicatesSearch()));
    ac->addAction(QLatin1String("find_duplicates"), duplicatesAction);
    ac->setDefaultShortcut(duplicatesAction, Qt::CTRL | Qt::Key_D);

    // -----------------------------------------------------------

#ifdef HAVE_MYSQLSUPPORT

    QAction* const databaseMigrationAction = new QAction(QIcon::fromTheme(QLatin1String("network-server-database")),
                                                         i18nc("@action: setup", "Database Migration..."), this);
    connect(databaseMigrationAction, SIGNAL(triggered()), this, SLOT(slotDatabaseMigration()));
    ac->addAction(QLatin1String("database_migration"), databaseMigrationAction);

#endif

    // -----------------------------------------------------------

    d->maintenanceAction = new QAction(QIcon::fromTheme(QLatin1String("run-build-prune")),
                                       i18nc("@action: setup", "Maintenance..."), this);
    connect(d->maintenanceAction, SIGNAL(triggered()), this, SLOT(slotMaintenance()));
    ac->addAction(QLatin1String("maintenance"), d->maintenanceAction);

    // -----------------------------------------------------------

    d->scanNewItemsAction = new QAction(QIcon::fromTheme(QLatin1String("view-refresh")),
                                        i18nc("@action: setup", "Scan for new Items"), this);
    connect(d->scanNewItemsAction, SIGNAL(triggered()), this, SLOT(slotScanNewItems()));
    ac->addAction(QLatin1String("scan_newitems"), d->scanNewItemsAction);

    // -----------------------------------------------------------

    QAction* const cameraAction = new QAction(i18nc("@action: setup", "Add Camera Manually..."), this);
    connect(cameraAction, SIGNAL(triggered()), this, SLOT(slotSetupCamera()));
    ac->addAction(QLatin1String("camera_add"), cameraAction);

    // -----------------------------------------------------------

    // Load Cameras -- do this before the createGUI so that the cameras
    // are plugged into the toolbar at startup

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18nc("@info: setup", "Loading cameras..."));
    }
    else
    {
        qApp->processEvents();
    }

    loadCameras();

    // Load Themes

    populateThemes();

    createGUI(xmlFile());

    registerPluginsActions();

    cleanupActions();

    // NOTE: see bug #252130 and #283281 : we need to disable these actions when BQM is running.
    // These connections must be done after loading color theme else theme menu cannot be plugged to Settings menu,

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->bqmAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddCurrentQueueAction, SLOT(setDisabled(bool)));

    connect(QueueMgrWindow::queueManagerWindow(), SIGNAL(signalBqmIsBusy(bool)),
            d->imageAddNewQueueAction, SLOT(setDisabled(bool)));
}

void DigikamApp::setupAccelerators()
{
    KActionCollection* const ac = actionCollection();

    // Action are added by <MainWindow> tag in ui.rc XML file

    QAction* const escapeAction = new QAction(i18nc("@action: setup", "Exit Preview Mode"), this);
    ac->addAction(QLatin1String("exit_preview_mode"), escapeAction);
    ac->setDefaultShortcut(escapeAction, Qt::Key_Escape);
    connect(escapeAction, SIGNAL(triggered()), this, SIGNAL(signalEscapePressed()));

    QAction* const nextImageAction = new QAction(i18nc("@action: setup", "Next Image"), this);
    nextImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    ac->addAction(QLatin1String("next_image"), nextImageAction);
    ac->setDefaultShortcut(nextImageAction, Qt::Key_Space);
    connect(nextImageAction, SIGNAL(triggered()), this, SIGNAL(signalNextItem()));

    QAction* const previousImageAction = new QAction(i18nc("@action: setup", "Previous Image"), this);
    previousImageAction->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    ac->addAction(QLatin1String("previous_image"), previousImageAction);
    ac->setDefaultShortcuts(previousImageAction, QList<QKeySequence>() << Qt::Key_Backspace << (Qt::SHIFT | Qt::Key_Space));
    connect(previousImageAction, SIGNAL(triggered()), this, SIGNAL(signalPrevItem()));

    QAction* const firstImageAction = new QAction(i18nc("@action: setup", "First Image"), this);
    ac->addAction(QLatin1String("first_image"), firstImageAction);
    ac->setDefaultShortcut(firstImageAction, Qt::CTRL | Qt::Key_Home);
    connect(firstImageAction, SIGNAL(triggered()), this, SIGNAL(signalFirstItem()));

    QAction* const lastImageAction = new QAction(i18nc("@action: setup", "Last Image"), this);
    ac->addAction(QLatin1String("last_image"), lastImageAction);
    ac->setDefaultShortcut(lastImageAction, Qt::CTRL | Qt::Key_End);
    connect(lastImageAction, SIGNAL(triggered()), this, SIGNAL(signalLastItem()));

    d->cutItemsAction = new QAction(i18nc("@action: setup", "Cu&t"), this);
    d->cutItemsAction->setIcon(QIcon::fromTheme(QLatin1String("edit-cut")));
    d->cutItemsAction->setWhatsThis(i18nc("@action: setup", "Cut selection to clipboard"));
    ac->addAction(QLatin1String("cut_album_selection"), d->cutItemsAction);

    // NOTE: shift+del keyboard shortcut must not be assigned to Cut action
    // else the shortcut for Delete permanently collides with secondary shortcut of Cut

    ac->setDefaultShortcut(d->cutItemsAction, Qt::CTRL | Qt::Key_X);
    connect(d->cutItemsAction, SIGNAL(triggered()), this, SIGNAL(signalCutAlbumItemsSelection()));

    d->copyItemsAction = buildStdAction(StdCopyAction, this, SIGNAL(signalCopyAlbumItemsSelection()), this);
    ac->addAction(QLatin1String("copy_album_selection"), d->copyItemsAction);

    d->pasteItemsAction = buildStdAction(StdPasteAction, this, SIGNAL(signalPasteAlbumItemsSelection()), this);
    ac->addAction(QLatin1String("paste_album_selection"), d->pasteItemsAction);

    // Labels shortcuts must be registered here to be saved in XML GUI files if user customize it.

    d->tagsActionManager->registerLabelsActions(ac);

    QAction* const editTitles = new QAction(i18nc("@action: setup", "Edit Titles"), this);
    ac->addAction(QLatin1String("edit_titles"), editTitles);
    ac->setDefaultShortcut(editTitles, Qt::ALT | Qt::SHIFT | Qt::Key_T);
    connect(editTitles, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateTitles()));

    QAction* const editComments = new QAction(i18nc("@action: setup", "Edit Comments"), this);
    ac->addAction(QLatin1String("edit_comments"), editComments);
    ac->setDefaultShortcut(editComments, Qt::ALT | Qt::SHIFT | Qt::Key_C);
    connect(editComments, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateComments()));

    QAction* const assignedTags = new QAction(i18nc("@action: setup", "Show Assigned Tags"), this);
    ac->addAction(QLatin1String("assigned_tags"), assignedTags);
    ac->setDefaultShortcut(assignedTags, Qt::ALT | Qt::SHIFT | Qt::Key_A);
    connect(assignedTags, SIGNAL(triggered()), d->view, SLOT(slotRightSideBarActivateAssignedTags()));
}

void DigikamApp::setupExifOrientationActions()
{
    KActionCollection* const ac       = actionCollection();
    d->imageExifOrientationActionMenu = new QMenu(i18nc("@action: setup", "Adjust Exif Orientation Tag"), this);
    ac->addAction(QLatin1String("image_set_exif_orientation"), d->imageExifOrientationActionMenu->menuAction());

    d->imageSetExifOrientation1Action = new QAction(i18nc("@action: normal exif orientation", "Normal"), this);
    d->imageSetExifOrientation1Action->setCheckable(true);
    d->imageSetExifOrientation2Action = new QAction(i18nc("@action: setup", "Flipped Horizontally"),               this);
    d->imageSetExifOrientation2Action->setCheckable(true);
    d->imageSetExifOrientation3Action = new QAction(i18nc("@action: setup", "Rotated Upside Down"),                this);
    d->imageSetExifOrientation3Action->setCheckable(true);
    d->imageSetExifOrientation4Action = new QAction(i18nc("@action: setup", "Flipped Vertically"),                 this);
    d->imageSetExifOrientation4Action->setCheckable(true);
    d->imageSetExifOrientation5Action = new QAction(i18nc("@action: setup", "Rotated Right / Horiz. Flipped"),     this);
    d->imageSetExifOrientation5Action->setCheckable(true);
    d->imageSetExifOrientation6Action = new QAction(i18nc("@action: setup", "Rotated Right"),                      this);
    d->imageSetExifOrientation6Action->setCheckable(true);
    d->imageSetExifOrientation7Action = new QAction(i18nc("@action: setup", "Rotated Right / Vert. Flipped"),      this);
    d->imageSetExifOrientation7Action->setCheckable(true);
    d->imageSetExifOrientation8Action = new QAction(i18nc("@action: setup", "Rotated Left"),                       this);
    d->imageSetExifOrientation8Action->setCheckable(true);

    d->exifOrientationActionGroup = new QActionGroup(d->imageExifOrientationActionMenu);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation1Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation2Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation3Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation4Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation5Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation6Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation7Action);
    d->exifOrientationActionGroup->addAction(d->imageSetExifOrientation8Action);
    d->imageSetExifOrientation1Action->setChecked(true);

    ac->addAction(QLatin1String("image_set_exif_orientation_normal"),                    d->imageSetExifOrientation1Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_flipped_horizontal"),        d->imageSetExifOrientation2Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_upside_down"),       d->imageSetExifOrientation3Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_flipped_vertically"),        d->imageSetExifOrientation4Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right_hor_flipped"), d->imageSetExifOrientation5Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right"),             d->imageSetExifOrientation6Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_right_ver_flipped"), d->imageSetExifOrientation7Action);
    ac->addAction(QLatin1String("image_set_exif_orientation_rotated_left"),              d->imageSetExifOrientation8Action);

    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation1Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation2Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation3Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation4Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation5Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation6Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation7Action);
    d->imageExifOrientationActionMenu->addAction(d->imageSetExifOrientation8Action);

    connect(d->imageSetExifOrientation1Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(1); });

    connect(d->imageSetExifOrientation2Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(2); });

    connect(d->imageSetExifOrientation3Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(3); });

    connect(d->imageSetExifOrientation4Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(4); });

    connect(d->imageSetExifOrientation5Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(5); });

    connect(d->imageSetExifOrientation6Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(6); });

    connect(d->imageSetExifOrientation7Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(7); });

    connect(d->imageSetExifOrientation8Action, &QAction::triggered,
            this, [this]() { d->view->slotImageExifOrientation(8); });
}

void DigikamApp::setupImageTransformActions()
{
    KActionCollection* const ac = actionCollection();

    d->imageRotateActionMenu = new QMenu(i18nc("@action: setup", "Rotate"), this);
    d->imageRotateActionMenu->setIcon(QIcon::fromTheme(QLatin1String("object-rotate-right")));

    QAction* const left = ac->addAction(QLatin1String("rotate_ccw"));
    left->setText(i18nc("@action: rotate image left", "Left"));
    ac->setDefaultShortcut(left, Qt::CTRL | Qt::SHIFT | Qt::Key_Left);
    connect(left, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(left);

    QAction* const right = ac->addAction(QLatin1String("rotate_cw"));
    right->setText(i18nc("@action: rotate image right", "Right"));
    ac->setDefaultShortcut(right, Qt::CTRL | Qt::SHIFT | Qt::Key_Right);
    connect(right, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageRotateActionMenu->addAction(right);

    ac->addAction(QLatin1String("image_rotate"), d->imageRotateActionMenu->menuAction());

    // -----------------------------------------------------------------------------------

    d->imageFlipActionMenu = new QMenu(i18nc("@action: setup", "Flip"), this);
    d->imageFlipActionMenu->setIcon(QIcon::fromTheme(QLatin1String("flip-horizontal")));

    QAction* const hori = ac->addAction(QLatin1String("flip_horizontal"));
    hori->setText(i18nc("@action: setup", "Horizontally"));
    ac->setDefaultShortcut(hori, Qt::CTRL | Qt::Key_Asterisk);
    connect(hori, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(hori);

    QAction* const verti = ac->addAction(QLatin1String("flip_vertical"));
    verti->setText(i18nc("@action: setup", "Vertically"));
    ac->setDefaultShortcut(verti, Qt::CTRL | Qt::Key_Slash);
    connect(verti, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));
    d->imageFlipActionMenu->addAction(verti);

    ac->addAction(QLatin1String("image_flip"), d->imageFlipActionMenu->menuAction());

    // -----------------------------------------------------------------------------------

    d->imageAutoExifActionMenu = new QAction(i18nc("@action: setup", "Auto Rotate/Flip Using Exif Information"), this);
    connect(d->imageAutoExifActionMenu, SIGNAL(triggered(bool)),
            this, SLOT(slotTransformAction()));

    ac->addAction(QLatin1String("image_transform_exif"), d->imageAutoExifActionMenu);
}

void DigikamApp::populateThemes()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18nc("@info: setup", "Loading themes..."));
    }
    else
    {
        qApp->processEvents();
    }

    ThemeManager::instance()->setThemeMenuAction(new QMenu(i18nc("@action: setup", "&Themes"), this));
    ThemeManager::instance()->registerThemeActions(this);
    ThemeManager::instance()->setCurrentTheme(ApplicationSettings::instance()->getCurrentTheme());

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

void DigikamApp::preloadWindows()
{
    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18nc("@info: setup", "Loading tools..."));
    }
    else
    {
        qApp->processEvents();
    }

    ImageWindow::imageWindow();
    QueueMgrWindow::queueManagerWindow();
    LightTableWindow::lightTableWindow();

    d->tagsActionManager->registerTagsActionCollections();
}

void DigikamApp::initGui()
{
    // Initialize Actions ---------------------------------------

    d->deleteAction->setEnabled(false);
    d->renameAction->setEnabled(false);
    d->propsEditAction->setEnabled(false);
    d->openInFileManagerAction->setEnabled(false);

    d->imageViewAction->setEnabled(false);
    d->imagePreviewAction->setEnabled(false);
    d->imageLightTableAction->setEnabled(false);
    d->imageAddLightTableAction->setEnabled(false);
    d->imageScanForFacesAction->setEnabled(false);
    d->imageFindSimilarAction->setEnabled(false);
    d->imageRenameAction->setEnabled(false);
    d->imageDeleteAction->setEnabled(false);
    d->imageExifOrientationActionMenu->setEnabled(false);
    d->openWithAction->setEnabled(false);
    d->imageAutoExifActionMenu->setEnabled(false);

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericMetadata, this))
    {
        ac->setEnabled(false);
    }

    d->albumSortAction->setCurrentItem((int)ApplicationSettings::instance()->getAlbumSortRole());
    d->imageSortAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSortOrder());
    d->imageSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSorting());
    d->imageSeparationAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSeparationMode()-1); // no action for enum 0
    d->imageSeparationSortOrderAction->setCurrentItem((int)ApplicationSettings::instance()->getImageSeparationSortOrder());
    d->allGroupsOpenAction->setChecked(ApplicationSettings::instance()->getAllGroupsOpen());
    d->recurseAlbumsAction->setChecked(ApplicationSettings::instance()->getRecurseAlbums());
    d->recurseTagsAction->setChecked(ApplicationSettings::instance()->getRecurseTags());
    d->showBarAction->setChecked(ApplicationSettings::instance()->getShowThumbbar());
    showMenuBarAction()->setChecked(!menuBar()->isHidden());                                                    // NOTE: workaround for bug #171080

    slotSwitchedToIconView();
}

} // namespace Digikam
