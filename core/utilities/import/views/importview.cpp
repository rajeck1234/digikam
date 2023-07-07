/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-26-07
 * Description : Main view for import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importview.h"

// Qt includes

#include <QApplication>
#include <QTimer>
#include <QShortcut>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "digikam_globals.h"
#include "dmessagebox.h"
#include "importui.h"
#include "importiconview.h"
#include "thumbnailsize.h"
#include "fileactionmngr.h"
#include "importsettings.h"
#include "sidebar.h"
#include "dzoombar.h"
#include "camitemsortsettings.h"

#ifdef HAVE_MARBLE
#   include "mapwidgetview.h"
#endif // HAVE_MARBLE

namespace Digikam
{

class Q_DECL_HIDDEN ImportView::Private
{
public:

    explicit Private()
      : thumbSize     (ThumbnailSize::Medium),
        dockArea      (nullptr),
        splitter      (nullptr),
        selectionTimer(nullptr),
        thumbSizeTimer(nullptr),
        parent        (nullptr),
        iconView      (nullptr),

#ifdef HAVE_MARBLE

        mapView(nullptr),

#endif // HAVE_MARBLE

        stackedView(nullptr),
        lastViewMode(ImportStackedView::PreviewCameraMode)
/*
        FIXME
        , filterWidget(0)
*/
    {
    }

    void addPageUpDownActions(ImportView* const q, QWidget* const w);

public:

    int                                thumbSize;

    QMainWindow*                       dockArea;

    SidebarSplitter*                   splitter;

    QTimer*                            selectionTimer;
    QTimer*                            thumbSizeTimer;

    ImportUI*                          parent;

    ImportIconView*                    iconView;

#ifdef HAVE_MARBLE

    MapWidgetView*                     mapView;

#endif // HAVE_MARBLE

    ImportStackedView*                 stackedView;
    ImportStackedView::StackedViewMode lastViewMode;
/*
    FIXME
    FilterSideBarWidget*               filterWidget;
*/
};

void ImportView::Private::addPageUpDownActions(ImportView* const q, QWidget* const w)
{
    defineShortcut(w, Qt::Key_PageDown, q, SLOT(slotNextItem()));
    defineShortcut(w, Qt::Key_Down,     q, SLOT(slotNextItem()));
    defineShortcut(w, Qt::Key_Right,    q, SLOT(slotNextItem()));

    defineShortcut(w, Qt::Key_PageUp,   q, SLOT(slotPrevItem()));
    defineShortcut(w, Qt::Key_Up,       q, SLOT(slotPrevItem()));
    defineShortcut(w, Qt::Key_Left,     q, SLOT(slotPrevItem()));
}

ImportView::ImportView(ImportUI* const ui, QWidget* const parent)
    : DHBox(parent),
      d(new Private)
{
    d->parent   = static_cast<ImportUI*>(ui);
    d->splitter = new SidebarSplitter;
    d->splitter->setFrameStyle(QFrame::NoFrame);
    d->splitter->setFrameShadow(QFrame::Plain);
    d->splitter->setFrameShape(QFrame::NoFrame);
    d->splitter->setOpaqueResize(false);
    d->splitter->setParent(this);

    // The dock area where the thumbnail bar is allowed to go.
    // TODO qmainwindow here?

    d->dockArea    = new QMainWindow(this, Qt::Widget);
    d->splitter->addWidget(d->dockArea);
    d->stackedView = new ImportStackedView(d->dockArea);
    d->stackedView->setViewMode(ImportStackedView::PreviewCameraMode); // call here, because the models need to be set first..
    d->dockArea->setCentralWidget(d->stackedView);
    d->stackedView->setDockArea(d->dockArea);

    d->iconView = d->stackedView->importIconView();

#ifdef HAVE_MARBLE

    d->mapView  = d->stackedView->mapWidgetView();

#endif // HAVE_MARBLE

    d->addPageUpDownActions(this, d->stackedView->importPreviewView());
    d->addPageUpDownActions(this, d->stackedView->thumbBar());

#ifdef HAVE_MEDIAPLAYER

    d->addPageUpDownActions(this, d->stackedView->mediaPlayerView());

#endif //HAVE_MEDIAPLAYER

    d->selectionTimer = new QTimer(this);
    d->selectionTimer->setSingleShot(true);
    d->selectionTimer->setInterval(75);

    d->thumbSizeTimer = new QTimer(this);
    d->thumbSizeTimer->setSingleShot(true);
    d->thumbSizeTimer->setInterval(300);

    setupConnections();

    loadViewState();
}

ImportView::~ImportView()
{
    saveViewState();
    delete d;
}

void ImportView::applySettings()
{
/*
    refreshView();
*/
}

void ImportView::refreshView()
{
/*
    d->rightSideBar->refreshTagsView();
*/
}

void ImportView::setupConnections()
{
    // -- ImportUI connections ----------------------------------

    connect(d->parent, SIGNAL(signalEscapePressed()),
            this, SLOT(slotEscapePreview()));

    connect(d->parent, SIGNAL(signalEscapePressed()),
            d->stackedView, SLOT(slotEscapePreview()));

    // Preview items while download.

    connect(d->parent, SIGNAL(signalPreviewRequested(CamItemInfo,bool)),
            this, SLOT(slotTogglePreviewMode(CamItemInfo,bool)));

    // -- IconView Connections -------------------------------------

    connect(d->iconView->model(), SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(slotImageSelected()));

    connect(d->iconView->model(), SIGNAL(layoutChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(selectionChanged()),
            this, SLOT(slotImageSelected()));

    connect(d->iconView, SIGNAL(previewRequested(CamItemInfo,bool)),
            this, SLOT(slotTogglePreviewMode(CamItemInfo,bool)));

    connect(d->iconView, SIGNAL(zoomOutStep()),
            this, SLOT(slotZoomOut()));

    connect(d->iconView, SIGNAL(zoomInStep()),
            this, SLOT(slotZoomIn()));

    // -- Preview image widget Connections ------------------------

    connect(d->stackedView, SIGNAL(signalNextItem()),
            this, SLOT(slotNextItem()));

    connect(d->stackedView, SIGNAL(signalPrevItem()),
            this, SLOT(slotPrevItem()));

    connect(d->stackedView, SIGNAL(signalViewModeChanged()),
            this, SLOT(slotViewModeChanged()));

    connect(d->stackedView, SIGNAL(signalEscapePreview()),
            this, SLOT(slotEscapePreview()));

    connect(d->stackedView, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    // -- FileActionMngr progress ---------------

    connect(FileActionMngr::instance(), SIGNAL(signalImageChangeFailed(QString,QStringList)),
            this, SLOT(slotImageChangeFailed(QString,QStringList)));

    // -- timers ---------------

    connect(d->selectionTimer, SIGNAL(timeout()),
            this, SLOT(slotDispatchImageSelected()));

    connect(d->thumbSizeTimer, SIGNAL(timeout()),
            this, SLOT(slotThumbSizeEffect()) );

    // -- Import Settings ----------------
/*
    connect(ImportSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));
*/
}

/*
void ImportView::connectIconViewFilter(FilterStatusBar* filterbar)
{
    ItemAlbumFilterModel* const model = d->iconView->imageAlbumFilterModel();

    connect(model, SIGNAL(filterMatches(bool)),
            filterbar, SLOT(slotFilterMatches(bool)));

    connect(model, SIGNAL(filterSettingsChanged(ItemFilterSettings)),
            filterbar, SLOT(slotFilterSettingsChanged(ItemFilterSettings)));

    connect(filterbar, SIGNAL(signalResetFilters()),
            d->filterWidget, SLOT(slotResetFilters()));

    connect(filterbar, SIGNAL(signalPopupFiltersView()),
            this, SLOT(slotPopupFiltersView()));
}

void ImportView::slotPopupFiltersView()
{
    d->rightSideBar->setActiveTab(d->filterWidget);
    d->filterWidget->setFocusToTextFilter();
}
*/

void ImportView::loadViewState()
{
/*
    TODO
    d->filterWidget->loadState();
*/
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Import MainWindow");

    // Restore the splitter

    d->splitter->restoreState(group);

    // Restore the thumbnail bar dock.

    QByteArray thumbbarState;
    thumbbarState = group.readEntry("ThumbbarState", thumbbarState);
    d->dockArea->restoreState(QByteArray::fromBase64(thumbbarState));

#ifdef HAVE_MARBLE

    d->mapView->loadState();

#endif // HAVE_MARBLE

}

void ImportView::saveViewState()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group("Import MainWindow");
/*
    TODO
    d->filterWidget->saveState();
*/

    // Save the splitter states.

    d->splitter->saveState(group);

    // Save the position and size of the thumbnail bar. The thumbnail bar dock
    // needs to be closed explicitly, because when it is floating and visible
    // (when the user is in image preview mode) when the layout is saved, it
    // also reappears when restoring the view, while it should always be hidden.

    d->stackedView->thumbBarDock()->close();
    group.writeEntry("ThumbbarState", d->dockArea->saveState().toBase64());

#ifdef HAVE_MARBLE

    d->mapView->saveState();

#endif // HAVE_MARBLE

}

CamItemInfo ImportView::camItemInfo(const QString& folder, const QString& file) const
{
    return d->iconView->camItemInfo(folder, file);
}

CamItemInfo& ImportView::camItemInfoRef(const QString& folder, const QString& file) const
{
    return d->iconView->camItemInfoRef(folder, file);
}

bool ImportView::hasImage(const CamItemInfo& info) const
{
    return d->iconView->importItemModel()->hasImage(info);
}

QList<QUrl> ImportView::allUrls() const
{
    return d->iconView->urls();
}

QList<QUrl> ImportView::selectedUrls() const
{
    return d->iconView->selectedUrls();
}

QList<CamItemInfo> ImportView::selectedCamItemInfos() const
{
    return d->iconView->selectedCamItemInfos();
}

QList<CamItemInfo> ImportView::allItems() const
{
    return d->iconView->camItemInfos();
}

void ImportView::setSelectedCamItemInfos(const CamItemInfoList& infos) const
{
    d->iconView->setSelectedCamItemInfos(infos);
}

int ImportView::downloadedCamItemInfos() const
{
    QList<CamItemInfo> infos = d->iconView->camItemInfos();
    int numberOfDownloaded   = 0;

    Q_FOREACH (const CamItemInfo& info, infos)
    {
        if (info.downloaded == CamItemInfo::DownloadedYes)
        {
            ++numberOfDownloaded; // cppcheck-suppress useStlAlgorithm
        }
    }

    return numberOfDownloaded;
}

bool ImportView::isSelected(const QUrl& url) const
{
    QList<QUrl> urlsList = selectedUrls();

    Q_FOREACH (const QUrl& selected, urlsList)
    {
        if (url == selected)
        {   // cppcheck-suppress useStlAlgorithm
            return true;
        }
    }

    return false;
}

void ImportView::slotFirstItem()
{
    d->iconView->toFirstIndex();
}

void ImportView::slotPrevItem()
{
    d->iconView->toPreviousIndex();
}

void ImportView::slotNextItem()
{
    d->iconView->toNextIndex();
}

void ImportView::slotLastItem()
{
    d->iconView->toLastIndex();
}

void ImportView::slotSelectItemByUrl(const QUrl& url)
{
    d->iconView->toIndex(url);
}

void ImportView::slotImageSelected()
{
    // delay to slotDispatchImageSelected

    d->selectionTimer->start();
    Q_EMIT signalSelectionChanged(d->iconView->numberOfSelectedIndexes());
}

void ImportView::slotDispatchImageSelected()
{
    // the list of CamItemInfos of currently selected items, currentItem first
    // since the iconView tracks the changes also while we are in map widget mode,
    // we can still pull the data from the iconView

    const CamItemInfoList& list      = d->iconView->selectedCamItemInfosCurrentFirst();
    const CamItemInfoList& allImages = d->iconView->camItemInfos();

    if (list.isEmpty())
    {
        d->stackedView->setPreviewItem();
        Q_EMIT signalImageSelected(list, allImages);
        Q_EMIT signalNewSelection(false);
        Q_EMIT signalNoCurrentItem();
    }
    else
    {
        CamItemInfo previousInfo;
        CamItemInfo nextInfo;

        if (d->stackedView->viewMode() != ImportStackedView::MapWidgetMode)
        {
            previousInfo = d->iconView->previousInfo(list.first());
            nextInfo     = d->iconView->nextInfo(list.first());
        }

        if ((d->stackedView->viewMode() != ImportStackedView::PreviewCameraMode) &&
            (d->stackedView->viewMode() != ImportStackedView::MapWidgetMode))
        {
            d->stackedView->setPreviewItem(list.first(), previousInfo, nextInfo);
        }

        Q_EMIT signalImageSelected(list, allImages);
        Q_EMIT signalNewSelection(true);
    }
}

double ImportView::zoomMin() const
{
    return d->stackedView->zoomMin();
}

double ImportView::zoomMax() const
{
    return d->stackedView->zoomMax();
}

void ImportView::setZoomFactor(double zoom)
{
    d->stackedView->setZoomFactorSnapped(zoom);
}

void ImportView::slotZoomFactorChanged(double zoom)
{
    toggleZoomActions();
    Q_EMIT signalZoomChanged(zoom);
}

void ImportView::setThumbSize(int size)
{
    if      (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        double z = DZoomBar::zoomFromSize(size, zoomMin(), zoomMax());
        setZoomFactor(z);
    }
    else if (d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode)
    {
        if      (size > ThumbnailSize::maxThumbsSize())
        {
            d->thumbSize = ThumbnailSize::maxThumbsSize();
        }
        else if (size < ThumbnailSize::Small)
        {
            d->thumbSize = ThumbnailSize::Small;
        }
        else
        {
            d->thumbSize = size;
        }

        Q_EMIT signalThumbSizeChanged(d->thumbSize);

        d->thumbSizeTimer->start();
    }
}

ThumbnailSize ImportView::thumbnailSize() const
{
    return ThumbnailSize(d->thumbSize);
}

void ImportView::slotThumbSizeEffect()
{
    d->iconView->setThumbnailSize(ThumbnailSize(d->thumbSize));
    toggleZoomActions();

    ImportSettings::instance()->setDefaultIconSize(d->thumbSize);
}

void ImportView::toggleZoomActions()
{
    if      (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->stackedView->maxZoom())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->stackedView->minZoom())
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else if (d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode)
    {
        d->parent->enableZoomMinusAction(true);
        d->parent->enableZoomPlusAction(true);

        if (d->thumbSize >= ThumbnailSize::maxThumbsSize())
        {
            d->parent->enableZoomPlusAction(false);
        }

        if (d->thumbSize <= ThumbnailSize::Small)
        {
            d->parent->enableZoomMinusAction(false);
        }
    }
    else
    {
        d->parent->enableZoomMinusAction(false);
        d->parent->enableZoomPlusAction(false);
    }
}

void ImportView::slotZoomIn()
{
    if      (d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode)
    {
        setThumbSize(d->thumbSize + ThumbnailSize::Step);
        toggleZoomActions();
        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        d->stackedView->increaseZoom();
    }
}

void ImportView::slotZoomOut()
{
    if      (d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode)
    {
        setThumbSize(d->thumbSize - ThumbnailSize::Step);
        toggleZoomActions();
        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        d->stackedView->decreaseZoom();
    }
}

void ImportView::slotZoomTo100Percents()
{
    if (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        d->stackedView->toggleFitToWindowOr100();
    }
}

void ImportView::slotFitToWindow()
{
    if      (d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode)
    {
        int nts = d->iconView->fitToWidthIcons();
        setThumbSize(nts);
        toggleZoomActions();
        Q_EMIT signalThumbSizeChanged(d->thumbSize);
    }
    else if (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        d->stackedView->fitToWindow();
    }
}

void ImportView::slotEscapePreview()
{
    if (d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode)
/*
        TODO
        || d->stackedView->viewMode() == ImportStackedView::WelcomePageMode)
*/
    {
        return;
    }

    // pass a null camera item info, because we want to fall back to the old
    // view mode

    slotTogglePreviewMode(CamItemInfo(), false);
}

void ImportView::slotMapWidgetView()
{
    d->stackedView->setViewMode(ImportStackedView::MapWidgetMode);
}

void ImportView::slotIconView()
{
    if (d->stackedView->viewMode() == ImportStackedView::PreviewImageMode)
    {
        Q_EMIT signalThumbSizeChanged(d->iconView->thumbnailSize().size());
    }

    // and switch to icon view

    d->stackedView->setViewMode(ImportStackedView::PreviewCameraMode);

    // make sure the next/previous buttons are updated

    slotImageSelected();
}

void ImportView::slotImagePreview()
{
    const int   currentPreviewMode = d->stackedView->viewMode();
    CamItemInfo currentInfo;

    if      (currentPreviewMode == ImportStackedView::PreviewCameraMode)
    {
        currentInfo = d->iconView->currentInfo();
    }

#ifdef HAVE_MARBLE

    // TODO: Implement MapWidget
    else if (currentPreviewMode == ImportStackedView::MapWidgetMode)
    {
        currentInfo = d->mapView->currentCamItemInfo();
    }

#endif // HAVE_MARBLE

    slotTogglePreviewMode(currentInfo, false);
}

/**
 * @brief This method toggles between IconView/MapWidgetView and ImportPreview modes, depending on the context.
 */
void ImportView::slotTogglePreviewMode(const CamItemInfo& info, bool downloadPreview)
{
    if (!d->parent->cameraUseUMSDriver())
    {
        return;
    }

    if (((d->stackedView->viewMode() == ImportStackedView::PreviewCameraMode) ||
         (d->stackedView->viewMode() == ImportStackedView::MapWidgetMode) || downloadPreview) &&
         !info.isNull())
    {
        d->lastViewMode      = d->stackedView->viewMode();
        CamItemInfo previous = CamItemInfo();

        if (!downloadPreview)
        {
            previous = d->iconView->previousInfo(info);
        }

        d->stackedView->setPreviewItem(info, previous, d->iconView->nextInfo(info));
    }
    else
    {
        // go back to either CameraViewMode or MapWidgetMode

        d->stackedView->setViewMode(d->lastViewMode);
    }

    if (!downloadPreview)
    {
        // make sure the next/previous buttons are updated

        slotImageSelected();
    }
}

void ImportView::slotViewModeChanged()
{
    toggleZoomActions();

    switch (d->stackedView->viewMode())
    {
        case ImportStackedView::PreviewCameraMode:
            Q_EMIT signalSwitchedToIconView();
            Q_EMIT signalThumbSizeChanged(d->iconView->thumbnailSize().size());
            break;

        case ImportStackedView::PreviewImageMode:
            Q_EMIT signalSwitchedToPreview();
            slotZoomFactorChanged(d->stackedView->zoomFactor());
            break;
/* TODO
        case ImportStackedView::WelcomePageMode:
            Q_EMIT signalSwitchedToIconView();
            break;
*/

        case ImportStackedView::MediaPlayerMode:
            Q_EMIT signalSwitchedToPreview();
            break;

        case ImportStackedView::MapWidgetMode:
            Q_EMIT signalSwitchedToMapView();

            // TODO: connect map view's zoom buttons to main status bar zoom buttons

            break;
    }
}

// TODO: Delete or implement this.
void ImportView::slotImageRename()
{
    d->iconView->rename();
}

void ImportView::slotSelectAll()
{
    d->iconView->selectAll();
}

void ImportView::slotSelectNone()
{
    d->iconView->clearSelection();
}

void ImportView::slotSelectInvert()
{
    d->iconView->invertSelection();
}

void ImportView::slotSortImagesBy(int sortBy)
{
    ImportSettings* const settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSortBy(sortBy);
    d->iconView->importFilterModel()->setSortRole((CamItemSortSettings::SortRole) sortBy);
}

void ImportView::slotSortImagesOrder(int order)
{
    ImportSettings* const settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSortOrder(order);
    d->iconView->importFilterModel()->setSortOrder((CamItemSortSettings::SortOrder) order);
}

void ImportView::slotSeparateImages(int categoryMode)
{
    ImportSettings* const settings = ImportSettings::instance();

    if (!settings)
    {
        return;
    }

    settings->setImageSeparationMode(categoryMode);
    d->iconView->importFilterModel()->setCategorizationMode((CamItemSortSettings::CategorizationMode) categoryMode);
}

void ImportView::toggleShowBar(bool b)
{
    d->stackedView->thumbBarDock()->showThumbBar(b);

    // See bug #319876 : force to reload current view mode to set thumbbar visibility properly.

    d->stackedView->setViewMode(viewMode());
}

void ImportView::scrollTo(const QString& folder, const QString& file)
{
    CamItemInfo info  = camItemInfo(folder, file);
    QModelIndex index = d->iconView->importFilterModel()->indexForCamItemInfo(info);
    d->iconView->scrollToRelaxed(index);
    d->iconView->setSelectedCamItemInfos(CamItemInfoList() << info);
}

void ImportView::slotImageChangeFailed(const QString& message, const QStringList& fileNames)
{
    if (fileNames.isEmpty())
    {
        return;
    }

    DMessageBox::showInformationList(QMessageBox::Critical,
                                     qApp->activeWindow(),
                                     qApp->applicationName(),
                                     message,
                                     fileNames);
}

bool ImportView::hasCurrentItem() const
{
    // We should actually get this directly from the selection model,
    // but the iconView is fine for now.

    return !d->iconView->currentInfo().isNull();
}

/*
void ImportView::slotImageExifOrientation(int orientation)
{
    FileActionMngr::instance()->setExifOrientation(d->iconView->selectedCamItemInfos(), orientation);
}
*/

ImportFilterModel* ImportView::importFilterModel() const
{
    return d->iconView->importFilterModel();
}

ImportStackedView::StackedViewMode ImportView::viewMode() const
{
    return d->stackedView->viewMode();
}

void ImportView::toggleFullScreen(bool set)
{
    d->stackedView->importPreviewView()->toggleFullScreen(set);
}

void ImportView::updateIconView()
{
    d->iconView->viewport()->update();
}

} // namespace Digikam
