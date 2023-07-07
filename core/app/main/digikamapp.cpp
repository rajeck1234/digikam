/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-16-10
 * Description : main digiKam interface implementation
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText:      2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText:      2013 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "digikamapp_p.h"

namespace Digikam
{

DigikamApp* DigikamApp::m_instance = nullptr;

DigikamApp::DigikamApp()
    : DXmlGuiWindow(nullptr),
      d            (new Private)
{
    setObjectName(QLatin1String("Digikam"));
    setConfigGroupName(ApplicationSettings::instance()->generalConfigGroupName());
    setFullScreenOptions(FS_ALBUMGUI);
    setXMLFile(QLatin1String("digikamui5.rc"));

    m_instance         = this;
    d->config          = KSharedConfig::openConfig();
    KConfigGroup group = d->config->group(configGroupName());

#ifdef HAVE_DBUS

    new DigikamAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QLatin1String("/Digikam"), this);
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.digikam-") +
                                                  QString::number(QCoreApplication::instance()->applicationPid()));
#endif

    // collection scan

    if (!CollectionScanner::databaseInitialScanDone())
    {
        ScanController::instance()->completeCollectionScanDeferFiles();
    }

    if (ApplicationSettings::instance()->getShowSplashScreen() &&
        !qApp->isSessionRestored())
    {
        d->splashScreen = new DSplashScreen();
        d->splashScreen->show();
    }

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Initializing..."));
    }
    else
    {
        qApp->processEvents();
    }

    // Ensure creation

    AlbumManager::instance();
    LoadingCacheInterface::initialize();
    IccSettings::instance()->loadAllProfilesProperties();
    MetaEngineSettings::instance();
    DMetadataSettings::instance();
    ProgressManager::instance();
    ThumbnailLoadThread::setDisplayingWidget(this);
    DIO::instance();
    LocalizeSettings::instance();
    NetworkManager::instance();

    connect(LocalizeSettings::instance(), &LocalizeSettings::signalOpenLocalizeSetup,
            this, [=]()
        {
            Setup::execLocalize(this);
        }
    );

    if (!ExifToolProcess::isCreated())
    {
        ExifToolThread* const exifToolThread = new ExifToolThread(qApp);
        exifToolThread->start();
    }

    // creation of the engine on first use - when drawing -
    // can take considerable time and cause a noticeable hang in the UI thread.

    QFontMetrics fm(font());
    fm.horizontalAdvance(QLatin1String("a"));

    connect(ApplicationSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    connect(IccSettings::instance(), SIGNAL(signalSettingsChanged()),
            this, SLOT(slotColorManagementOptionsChanged()));

    d->cameraMenu      = new QMenu(this);
    d->usbMediaMenu    = new QMenu(this);
    d->cardReaderMenu  = new QMenu(this);
    d->quickImportMenu = new QMenu(this);

    d->cameraList = new CameraList(this, QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                         QLatin1String("/cameras.xml"));

    connect(d->cameraList, SIGNAL(signalCameraAdded(CameraType*)),
            this, SLOT(slotCameraAdded(CameraType*)));

    connect(d->cameraList, SIGNAL(signalCameraRemoved(QAction*)),
            this, SLOT(slotCameraRemoved(QAction*)));

    d->modelCollection = new DModelFactory;

    // This manager must be created after collection setup and before accelerators setup.

    d->tagsActionManager = new TagsActionMngr(this);

    // Load plugins

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Load Plugins..."));
    }
    else
    {
        qApp->processEvents();
    }

    DPluginLoader* const dpl = DPluginLoader::instance();
    dpl->init();

    // First create everything, then connect.
    // Otherwise some items may send signals and the slots can try
    // to access items which were not created yet.

    setupView();
    setupAccelerators();
    setupActions();
    setupStatusBar();

    initGui();

    setupViewConnections();
    applyMainWindowSettings(group);
    slotColorManagementOptionsChanged();

    // Check ICC profiles repository availability

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Checking ICC repository..."));
    }
    else
    {
        qApp->processEvents();
    }

    d->validIccPath = SetupICC::iccRepositoryIsValid();

    // Clean up database if enabled in the settings

    if (ApplicationSettings::instance()->getCleanAtStart() &&
        CollectionScanner::databaseInitialScanDone())
    {
        if (d->splashScreen)
        {
            d->splashScreen->setMessage(i18n("Clean up Database..."));
        }
        else
        {
            qApp->processEvents();
        }

        QEventLoop loop;

        DbCleaner* const tool = new DbCleaner(false, false);

        connect(tool, SIGNAL(signalComplete()),
                &loop, SLOT(quit()));

        tool->start();
        loop.exec();
    }

    // Read albums from database

    if (d->splashScreen)
    {
        d->splashScreen->setMessage(i18n("Loading albums..."));
    }
    else
    {
        qApp->processEvents();
    }

    if (CoreDbAccess().backend()->isOpen())
    {
        AlbumManager::instance()->startScan();
    }

    // Setting the initial menu options after all tools have been loaded

    QList<Album*> albumList = AlbumManager::instance()->currentAlbums();
    d->view->slotAlbumSelected(albumList);

    // preload additional windows

    preloadWindows();

    readFullScreenSettings(group);

#ifdef HAVE_KFILEMETADATA

    // Create BalooWrap object, because it need to register a listener
    // to update digiKam data when changes in Baloo occur
    BalooWrap* const baloo = BalooWrap::instance();
    Q_UNUSED(baloo);

#endif //HAVE_KFILEMETADATA

    setAutoSaveSettings(group, true);

    LoadSaveThread::setInfoProvider(new DatabaseLoadSaveFileInfoProvider);

    setupSelectToolsAction();
}

DigikamApp::~DigikamApp()
{
    ProgressManager::instance()->slotAbortAll();

    ItemAttributesWatch::shutDown();

    // Close and delete image editor instance.

    if (ImageWindow::imageWindowCreated())
    {
        ImageWindow::imageWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        ImageWindow::imageWindow()->close();
        qApp->processEvents();
    }

    // Close and delete light table instance.

    if (LightTableWindow::lightTableWindowCreated())
    {
        LightTableWindow::lightTableWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        LightTableWindow::lightTableWindow()->close();
        qApp->processEvents();
    }

    // Close and delete Batch Queue Manager instance.

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        QueueMgrWindow::queueManagerWindow()->setAttribute(Qt::WA_DeleteOnClose, true);
        QueueMgrWindow::queueManagerWindow()->close();
        qApp->processEvents();
    }

    // Close and delete Tags Manager instance.

    if (TagsManager::isCreated())
    {
        TagsManager::instance()->setAttribute(Qt::WA_DeleteOnClose, true);
        TagsManager::instance()->close();
    }

    if (MetadataHubMngr::isCreated())
    {
        delete MetadataHubMngr::internalPtr;
    }

#ifdef HAVE_KFILEMETADATA

    if (BalooWrap::isCreated())
    {
        delete BalooWrap::internalPtr;
    }

#endif

    delete d->view;
    d->view = nullptr;

    DPluginLoader::instance()->cleanUp();

    ApplicationSettings::instance()->setRecurseAlbums(d->recurseAlbumsAction->isChecked());
    ApplicationSettings::instance()->setRecurseTags(d->recurseTagsAction->isChecked());
    ApplicationSettings::instance()->setShowThumbbar(d->showBarAction->isChecked());
    ApplicationSettings::instance()->saveSettings();

    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();
    ItemAttributesWatch::cleanUp();
    AlbumThumbnailLoader::instance()->cleanUp();
    ThumbnailLoadThread::cleanUp();
    LoadingCacheInterface::cleanUp();
    DIO::cleanUp();

    // close database server

    DatabaseServerStarter::instance()->stopServerManagerProcess();

    delete d->modelCollection;

    m_instance = nullptr;

    delete d;
}

DigikamApp* DigikamApp::instance()
{
    return m_instance;
}

ItemIconView* DigikamApp::view() const
{
    return d->view;
}

void DigikamApp::show()
{
    // Remove Splashscreen.

    if (d->splashScreen)
    {
        d->splashScreen->finish(this);
    }
    else
    {
        qApp->processEvents();
    }

    // Display application window.

    KMainWindow::show();

    // Report errors from ICC repository path.

    if (!d->validIccPath)
    {
        QString message = i18n("<p>The ICC profiles folder seems to be invalid.</p>"
                               "<p>If you want to try setting it again, choose \"Yes\" here, otherwise "
                               "choose \"No\", and the \"Color Management\" feature "
                               "will be disabled until you solve this issue.</p>");

        if (QMessageBox::warning(this, qApp->applicationName(), message, QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
        {
            if (!setupICC())
            {
                d->config->group(QLatin1String("Color Management")).writeEntry(QLatin1String("EnableCM"), false);
                d->config->sync();
            }
        }
        else
        {
            d->config->group(QLatin1String("Color Management")).writeEntry(QLatin1String("EnableCM"), false);
            d->config->sync();
        }
    }

    // Init album icon view zoom factor.

    ApplicationSettings* const settings = ApplicationSettings::instance();

    slotThumbSizeChanged(settings->getDefaultIconSize());
    slotZoomSliderChanged(settings->getDefaultIconSize());
    d->autoShowZoomToolTip = true;

    // Enable finished the collection scan as deferred process

    if (settings->getScanAtStart()                  ||
        !CollectionScanner::databaseInitialScanDone())
    {
        NewItemsFinder* const tool = new NewItemsFinder(NewItemsFinder::ScanDeferredFiles);

        if (settings->getDetectFacesInNewImages())
        {
            connect(tool, SIGNAL(signalComplete()),
                    this, SLOT(slotDetectFaces()));
        }

        QTimer::singleShot(1000, tool, SLOT(start()));
    }

    if (d->splashScreen)
    {
        delete d->splashScreen;
        d->splashScreen = nullptr;
    }
}

void DigikamApp::restoreSession()
{
    // TODO: show and restore ImageEditor, Lighttable, and Batch Queue Manager main windows

    if (qApp->isSessionRestored())
    {
        int n = 1;

        while (KMainWindow::canBeRestored(n))
        {
            const QString className = KMainWindow::classNameOfToplevel(n);

            if (className == QLatin1String(Digikam::DigikamApp::staticMetaObject.className()))
            {
                restore(n, false);
                break;
            }

            ++n;
        }
    }
}

void DigikamApp::closeEvent(QCloseEvent* e)
{
    // may show a progress dialog to finish actions

    FileActionMngr::instance()->requestShutDown();

    // may show a progress dialog to apply pending metadata

    if (MetadataHubMngr::isCreated())
    {
        MetadataHubMngr::instance()->requestShutDown();
    }

    DXmlGuiWindow::closeEvent(e);
}

bool DigikamApp::queryClose()
{
    bool ret = true;

    if (ImageWindow::imageWindowCreated())
    {
        ret &= ImageWindow::imageWindow()->queryClose();
    }

    if (QueueMgrWindow::queueManagerWindowCreated())
    {
        ret &= QueueMgrWindow::queueManagerWindow()->queryClose();
    }

    return ret;
}

void DigikamApp::enableZoomPlusAction(bool val)
{
    d->zoomPlusAction->setEnabled(val);
}

void DigikamApp::enableZoomMinusAction(bool val)
{
    d->zoomMinusAction->setEnabled(val);
}

void DigikamApp::enableAlbumBackwardHistory(bool enable)
{
    d->backwardActionMenu->setEnabled(enable);
}

void DigikamApp::enableAlbumForwardHistory(bool enable)
{
    d->forwardActionMenu->setEnabled(enable);
}

void DigikamApp::slotAboutToShowBackwardMenu()
{
    d->backwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getBackwardHistory(titles);

    for (int i = 0 ; i < titles.size() ; ++i)
    {
        QAction* const action = d->backwardActionMenu->menu()->addAction(titles.at(i));
        int id                = i + 1;

        connect(action, &QAction::triggered,
                this, [this, id]() { d->view->slotAlbumHistoryBack(id); });
    }
}

void DigikamApp::slotAboutToShowForwardMenu()
{
    d->forwardActionMenu->menu()->clear();
    QStringList titles;
    d->view->getForwardHistory(titles);

    for (int i = 0 ; i < titles.size() ; ++i)
    {
        QAction* const action = d->forwardActionMenu->menu()->addAction(titles.at(i));
        int id                = i + 1;

        connect(action, &QAction::triggered,
                this, [this, id]() { d->view->slotAlbumHistoryForward(id); });
    }
}

void DigikamApp::slotAlbumSelected(Album* album)
{
    if (album && !album->isTrashAlbum())
    {
        PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

        if (album->type() != Album::PHYSICAL || !palbum)
        {
            // Rules if not Physical album.

            d->deleteAction->setEnabled(false);
            d->renameAction->setEnabled(false);
            d->propsEditAction->setEnabled(false);
            d->openInFileManagerAction->setEnabled(false);
            d->newAction->setEnabled(false);
            d->writeAlbumMetadataAction->setEnabled(true);
            d->readAlbumMetadataAction->setEnabled(true);

            d->pasteItemsAction->setEnabled(!album->isRoot());

            d->selectInvertAction->setEnabled(!album->isRoot());
            d->selectNoneAction->setEnabled(!album->isRoot());
            d->selectAllAction->setEnabled(!album->isRoot());

            // Special case if Tag album.

            bool enabled = (
                            (album->type() == Album::TAG)                       &&
                            !album->isRoot()                                    &&
                            (album->id() != FaceTags::unconfirmedPersonTagId()) &&
                            (album->id() != FaceTags::unknownPersonTagId())
                           );

            d->newTagAction->setEnabled(enabled);
            d->deleteTagAction->setEnabled(enabled);
            d->editTagAction->setEnabled(enabled);
        }
        else
        {
            // Rules if Physical album.

            // We have either the abstract root album,
            // the album root album for collection base dirs, or normal albums.

            bool isRoot          = palbum->isRoot();
            bool isAlbumRoot     = palbum->isAlbumRoot();
            bool isNormalAlbum   = !isRoot && !isAlbumRoot;

            d->deleteAction->setEnabled(isNormalAlbum);
            d->renameAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->propsEditAction->setEnabled(isNormalAlbum);
            d->openInFileManagerAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->newAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->writeAlbumMetadataAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->readAlbumMetadataAction->setEnabled(isNormalAlbum || isAlbumRoot);

            d->pasteItemsAction->setEnabled(isNormalAlbum || isAlbumRoot);

            d->selectInvertAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->selectNoneAction->setEnabled(isNormalAlbum || isAlbumRoot);
            d->selectAllAction->setEnabled(isNormalAlbum || isAlbumRoot);
        }
    }
    else
    {
        // Rules if album is trash or no current album.

        d->deleteAction->setEnabled(false);
        d->renameAction->setEnabled(false);
        d->propsEditAction->setEnabled(false);
        d->openInFileManagerAction->setEnabled(false);
        d->newAction->setEnabled(false);
        d->writeAlbumMetadataAction->setEnabled(false);
        d->readAlbumMetadataAction->setEnabled(false);

        d->pasteItemsAction->setEnabled(false);
        d->copyItemsAction->setEnabled(false);
        d->cutItemsAction->setEnabled(false);

        d->selectInvertAction->setEnabled(false);
        d->selectNoneAction->setEnabled(false);
        d->selectAllAction->setEnabled(false);

        d->newTagAction->setEnabled(false);
        d->deleteTagAction->setEnabled(false);
        d->editTagAction->setEnabled(false);
    }
}

void DigikamApp::slotImageSelected(const ItemInfoList& selection, const ItemInfoList& listAll)
{
    qint64 listAllFileSize               = 0;
    qint64 selectionFileSize             = 0;

    int numOfImagesInModel               = 0;
    int numImagesWithGrouped             = listAll.count();
    int numImagesWithoutGrouped          = d->view->allUrls(false).count();

    ItemInfoList selectionWithoutGrouped = d->view->selectedInfoList(true, false);

    Q_FOREACH (const ItemInfo& info, selection)
    {
        // cppcheck-suppress useStlAlgorithm
        selectionFileSize += info.fileSize();
    }

    Q_FOREACH (const ItemInfo& info, listAll)
    {
        // cppcheck-suppress useStlAlgorithm
        listAllFileSize += info.fileSize();
    }

    Album* const album = d->view->currentAlbum();

    if (album && (album->type() == Album::PHYSICAL))
    {
        numOfImagesInModel = d->view->itemCount();
    }

    QString statusBarSelectionText;
    QString statusBarSelectionToolTip;

    switch (selection.count())
    {
        case 0:
        {
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = i18np("No item selected (%1 item)",
                                               "No item selected (%1 items)",
                                               numImagesWithoutGrouped);
                break;
            }

            statusBarSelectionText    = i18np("No item selected (%1 [%2] item)",
                                              "No item selected (%1 [%2] items)",
                                              numImagesWithoutGrouped,
                                              numImagesWithGrouped);

            statusBarSelectionToolTip = i18np("No item selected (%1 item. With grouped items: %2)",
                                              "No item selected (%1 items. With grouped items: %2)",
                                              numImagesWithoutGrouped,
                                              numImagesWithGrouped);
            break;
        }

        default:
        {
            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = i18np("1/%2 item selected (%3/%4)",
                                               "%1/%2 items selected (%3/%4)",
                                               selection.count(),
                                               numImagesWithoutGrouped,
                                               ItemPropertiesTab::humanReadableBytesCount(selectionFileSize),
                                               ItemPropertiesTab::humanReadableBytesCount(listAllFileSize));
                break;
            }

            if (selectionWithoutGrouped.count() > 1)
            {
                if (selection.count() == selectionWithoutGrouped.count())
                {
                    statusBarSelectionText    = i18np("1/%2 [%3] item selected (%4/%5)",
                                                      "%1/%2 [%3] items selected (%4/%5)",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      numImagesWithGrouped,
                                                      ItemPropertiesTab::humanReadableBytesCount(selectionFileSize),
                                                      ItemPropertiesTab::humanReadableBytesCount(listAllFileSize));

                    statusBarSelectionToolTip = i18np("1/%2 item selected. Total with grouped items: %3",
                                                      "%1/%2 items selected. Total with grouped items: %3",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      numImagesWithGrouped);
                }
                else
                {
                    statusBarSelectionText    = i18np("1/%2 [%3/%4] item selected (%5/%6)",
                                                      "%1/%2 [%3/%4] items selected (%5/%6)",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      selection.count(),
                                                      numImagesWithGrouped,
                                                      ItemPropertiesTab::humanReadableBytesCount(selectionFileSize),
                                                      ItemPropertiesTab::humanReadableBytesCount(listAllFileSize));

                    statusBarSelectionToolTip = i18np("1/%2 item selected. With grouped items: %3/%4",
                                                      "%1/%2 items selected. With grouped items: %3/%4",
                                                      selectionWithoutGrouped.count(),
                                                      numImagesWithoutGrouped,
                                                      selection.count(),
                                                      numImagesWithGrouped);
                }

                break;
            }

#if __GNUC__ >= 7   // krazy:exclude=cpp

            // no break; is completely intentional, arriving here is equivalent to case 1:
            [[fallthrough]];

#endif
        }

        case 1:
        {
            if (!selectionWithoutGrouped.isEmpty())
            {
                slotSetCheckedExifOrientationAction(selectionWithoutGrouped.first());
            }

            int index = listAll.indexOf(selection.first()) + 1;

            if (numImagesWithGrouped == numImagesWithoutGrouped)
            {
                statusBarSelectionText = selection.first().fileUrl().fileName()
                                            + i18n(" (%1 of %2)",
                                                   index, numImagesWithoutGrouped);
            }
            else
            {
                int indexWithoutGrouped = 0;

                if (!selectionWithoutGrouped.isEmpty())
                {
                    indexWithoutGrouped = d->view->allInfo(false).indexOf(selectionWithoutGrouped.first()) + 1;
                }

                statusBarSelectionText
                        = selection.first().fileUrl().fileName()
                          + i18n(" (%1 of %2 [%3] )", indexWithoutGrouped,
                                 numImagesWithoutGrouped, numImagesWithGrouped);
                statusBarSelectionToolTip
                        = selection.first().fileUrl().fileName()
                          + i18n(" (%1 of %2. Total with grouped items: %3)", indexWithoutGrouped,
                                 numImagesWithoutGrouped, numImagesWithGrouped);
            }

            break;
        }
    }

    if (numImagesWithGrouped < numOfImagesInModel)
    {
        statusBarSelectionText += QLatin1String(" - ");
        statusBarSelectionText += i18np("%1 item hidden", "%1 items hidden",
                                        numOfImagesInModel - numImagesWithGrouped);
    }

    d->statusLabel->setAdjustedText(statusBarSelectionText);
    d->statusLabel->setToolTip(statusBarSelectionToolTip);
}

void DigikamApp::slotTrashSelectionChanged(const QString& text)
{
    d->statusLabel->setAdjustedText(text);
    d->statusLabel->setToolTip(text);
}

void DigikamApp::slotSelectionChanged(int selectionCount)
{
    // The preview can either be activated when only one image is selected,
    // or if multiple images are selected, but one image is the 'current image'.

    bool hasAtLeastCurrent = (selectionCount == 1) || ((selectionCount > 0) && d->view->hasCurrentItem());

    d->imagePreviewAction->setEnabled(hasAtLeastCurrent);
    d->imageViewAction->setEnabled(hasAtLeastCurrent);
    d->imageScanForFacesAction->setEnabled(selectionCount > 0);
    d->imageFindSimilarAction->setEnabled(selectionCount == 1);
    d->imageRenameAction->setEnabled(selectionCount > 0);
    d->imageLightTableAction->setEnabled(selectionCount > 0);
    d->imageAddLightTableAction->setEnabled(selectionCount > 0);
    d->imageAddCurrentQueueAction->setEnabled((selectionCount > 0) && !QueueMgrWindow::queueManagerWindow()->isBusy());
    d->imageAddNewQueueAction->setEnabled((selectionCount > 0) && !QueueMgrWindow::queueManagerWindow()->isBusy());
    d->imageWriteMetadataAction->setEnabled(selectionCount > 0);
    d->imageReadMetadataAction->setEnabled(selectionCount > 0);
    d->imageDeleteAction->setEnabled(selectionCount > 0);
    d->imageRotateActionMenu->setEnabled(selectionCount > 0);
    d->imageFlipActionMenu->setEnabled(selectionCount > 0);
    d->imageExifOrientationActionMenu->setEnabled(selectionCount > 0);
    d->moveSelectionToAlbumAction->setEnabled(selectionCount > 0);
    d->copySelectionToAction->setEnabled(selectionCount > 0);
    d->cutItemsAction->setEnabled(selectionCount > 0);
    d->copyItemsAction->setEnabled(selectionCount > 0);
    d->openWithAction->setEnabled(selectionCount > 0);
    d->imageAutoExifActionMenu->setEnabled(selectionCount > 0);

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericMetadata, this))
    {
        ac->setEnabled(selectionCount > 0);
    }

    if (selectionCount > 0)
    {
        d->imageWriteMetadataAction->setText(i18np("Write Metadata to File",
                                                   "Write Metadata to Selected Files", selectionCount));
        d->imageReadMetadataAction->setText(i18np("Reread Metadata From File",
                                                  "Reread Metadata From Selected Files", selectionCount));

        slotResetExifOrientationActions();
    }
}

void DigikamApp::slotExit()
{
    close();
}

void DigikamApp::slotDBStat()
{
    showDigikamDatabaseStat();
}

void DigikamApp::slotRecurseAlbums(bool checked)
{
    d->view->setRecurseAlbums(checked);
}

void DigikamApp::slotRecurseTags(bool checked)
{
    d->view->setRecurseTags(checked);
}

void DigikamApp::slotAllGroupsOpen(bool checked)
{
    d->view->setAllGroupsOpen(checked);
}

void DigikamApp::slotZoomSliderChanged(int size)
{
    d->view->setThumbSize(size);
}

void DigikamApp::slotThumbSizeChanged(int size)
{
    d->zoomBar->setThumbsSize(size);

    if (!fullScreenIsActive() && d->autoShowZoomToolTip)
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void DigikamApp::slotZoomChanged(double zoom)
{
    double zmin = d->view->zoomMin();
    double zmax = d->view->zoomMax();
    d->zoomBar->setZoom(zoom, zmin, zmax);

    if (!fullScreenIsActive() && d->autoShowZoomToolTip)
    {
        d->zoomBar->triggerZoomTrackerToolTip();
    }
}

void DigikamApp::slotToggleShowBar()
{
    d->view->toggleShowBar(d->showBarAction->isChecked());
}

void DigikamApp::moveEvent(QMoveEvent*)
{
    Q_EMIT signalWindowHasMoved();
}

void DigikamApp::slotTransformAction()
{
    if      (sender()->objectName() == QLatin1String("rotate_ccw"))
    {
        d->view->imageTransform(MetaEngineRotation::Rotate270);
    }
    else if (sender()->objectName() == QLatin1String("rotate_cw"))
    {
        d->view->imageTransform(MetaEngineRotation::Rotate90);
    }
    else if (sender()->objectName() == QLatin1String("flip_horizontal"))
    {
        d->view->imageTransform(MetaEngineRotation::FlipHorizontal);
    }
    else if (sender()->objectName() == QLatin1String("flip_vertical"))
    {
        d->view->imageTransform(MetaEngineRotation::FlipVertical);
    }
    else if (sender()->objectName() == QLatin1String("image_transform_exif"))
    {
        // special value for FileActionMngr

        d->view->imageTransform(MetaEngineRotation::NoTransformation);
    }
}

void DigikamApp::slotResetExifOrientationActions()
{
    d->imageSetExifOrientation1Action->setChecked(false);
    d->imageSetExifOrientation2Action->setChecked(false);
    d->imageSetExifOrientation3Action->setChecked(false);
    d->imageSetExifOrientation4Action->setChecked(false);
    d->imageSetExifOrientation5Action->setChecked(false);
    d->imageSetExifOrientation6Action->setChecked(false);
    d->imageSetExifOrientation7Action->setChecked(false);
    d->imageSetExifOrientation8Action->setChecked(false);
}

void DigikamApp::slotSetCheckedExifOrientationAction(const ItemInfo& info)
{
/*
    QScopedPointer<DMetadata> meta(new DMetadata(info.fileUrl().toLocalFile()));
    int orientation = (meta->isEmpty()) ? 0 : meta->getItemOrientation();
*/
    int orientation = info.orientation();

    switch (orientation)
    {
        case 1:
            d->imageSetExifOrientation1Action->setChecked(true);
            break;

        case 2:
            d->imageSetExifOrientation2Action->setChecked(true);
            break;

        case 3:
            d->imageSetExifOrientation3Action->setChecked(true);
            break;

        case 4:
            d->imageSetExifOrientation4Action->setChecked(true);
            break;

        case 5:
            d->imageSetExifOrientation5Action->setChecked(true);
            break;

        case 6:
            d->imageSetExifOrientation6Action->setChecked(true);
            break;

        case 7:
            d->imageSetExifOrientation7Action->setChecked(true);
            break;

        case 8:
            d->imageSetExifOrientation8Action->setChecked(true);
            break;

        default:
            slotResetExifOrientationActions();
            break;
    }
}

void DigikamApp::showSideBars(bool visible)
{
    visible ? d->view->showSideBars()
            : d->view->hideSideBars();
}

void DigikamApp::slotToggleLeftSideBar()
{
    d->view->toggleLeftSidebar();
}

void DigikamApp::slotToggleRightSideBar()
{
    d->view->toggleRightSidebar();
}

void DigikamApp::slotPreviousLeftSideBarTab()
{
    d->view->previousLeftSideBarTab();
}

void DigikamApp::slotNextLeftSideBarTab()
{
    d->view->nextLeftSideBarTab();
}

void DigikamApp::slotNextRightSideBarTab()
{
    d->view->nextRightSideBarTab();
}

void DigikamApp::slotPreviousRightSideBarTab()
{
    d->view->previousRightSideBarTab();
}

void DigikamApp::showThumbBar(bool visible)
{
    view()->toggleShowBar(visible);
}

bool DigikamApp::thumbbarVisibility() const
{
    return d->showBarAction->isChecked();
}

void DigikamApp::slotSwitchedToPreview()
{
    d->zoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->imagePreviewAction->setChecked(true);
    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToIconView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageIconViewAction->setChecked(true);
    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToMapView()
{
    // TODO: Link to map view's zoom actions

    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);

#ifdef HAVE_MARBLE

    d->imageMapViewAction->setChecked(true);

#endif // HAVE_MARBLE

    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToTableView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    d->imageTableViewAction->setChecked(true);
    customizedTrashView(true);
    toggleShowBar();
}

void DigikamApp::slotSwitchedToTrashView()
{
    d->zoomBar->setBarMode(DZoomBar::ThumbsSizeCtrl);
    customizedTrashView(false);
    toggleShowBar();
}

void DigikamApp::customizedFullScreenMode(bool set)
{
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    showStatusBarAction()->setEnabled(!set);

    set ? d->showBarAction->setEnabled(false)
        : toggleShowBar();

    d->view->toggleFullScreen(set);
}

void DigikamApp::customizedTrashView(bool set)
{
    d->imageTableViewAction->setEnabled(set);
    d->imageIconViewAction->setEnabled(set);

#ifdef HAVE_MARBLE

    d->imageMapViewAction->setEnabled(set);

#endif

    d->imagePreviewAction->setEnabled(set);
    d->bqmAction->setEnabled(set);
    d->ltAction->setEnabled(set);
    d->ieAction->setEnabled(set);

    d->imageSeparationSortOrderAction->setEnabled(set);
    d->imageSeparationAction->setEnabled(set);
    d->imageSortOrderAction->setEnabled(set);
    d->imageSortAction->setEnabled(set);
    d->albumSortAction->setEnabled(set);

    d->zoomFitToWindowAction->setEnabled(set);
    d->recurseAlbumsAction->setEnabled(set);
    d->recurseTagsAction->setEnabled(set);
    d->refreshAction->setEnabled(set);

    Q_FOREACH (DPluginAction* const ac, DPluginLoader::instance()->pluginsActions(DPluginAction::GenericView, this))
    {
        ac->setEnabled(set);
    }
}

void DigikamApp::toggleShowBar()
{
    switch (d->view->viewMode())
    {
        case StackedView::PreviewImageMode:
        case StackedView::MediaPlayerMode:
            d->showBarAction->setEnabled(true);
            break;

        default:
            d->showBarAction->setEnabled(false);
            break;
    }
}

void DigikamApp::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void DigikamApp::slotOnlineVersionCheck()
{
    Setup::onlineVersionCheck();
}

void DigikamApp::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
}

void DigikamApp::slotColorManagementOptionsChanged()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    ThumbnailLoadThread::setDisplayingWidget(this);

    d->viewCMViewAction->blockSignals(true);
    d->viewCMViewAction->setEnabled(settings.enableCM);
    d->viewCMViewAction->setChecked(settings.useManagedPreviews);
    d->viewCMViewAction->blockSignals(false);
}

DInfoInterface* DigikamApp::infoIface(DPluginAction* const ac)
{
    ApplicationSettings::OperationType aset = ApplicationSettings::Unspecified;

    switch (ac->actionCategory())
    {
        case DPluginAction::GenericExport:
        case DPluginAction::GenericImport:
            aset = ApplicationSettings::ImportExport;
            break;

        case DPluginAction::GenericMetadata:
            aset = ApplicationSettings::Metadata;
            break;

        case DPluginAction::GenericTool:
            aset = ApplicationSettings::Tools;
            break;

        case DPluginAction::GenericView:
            aset = ApplicationSettings::Slideshow;
            break;

        default:
            break;
    }

    DBInfoIface* const iface = new DBInfoIface(this, QList<QUrl>(), aset);

    connect(iface, SIGNAL(signalImportedImage(QUrl)),
            this, SLOT(slotImportedImagefromScanner(QUrl)));

    if (aset == ApplicationSettings::Slideshow)
    {
        connect(iface, SIGNAL(signalLastItemUrl(QUrl)),
                d->view, SLOT(slotSetCurrentUrlWhenAvailable(QUrl)));
    }

    return iface;
}

} // namespace Digikam
