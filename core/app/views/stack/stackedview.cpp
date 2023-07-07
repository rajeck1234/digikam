/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-06-13
 * Description : A widget stack to embedded album content view
 *               or the current image preview.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "stackedview.h"

// Qt includes

#include <QWidget>

// KDE includes

#include <klocalizedstring.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "applicationsettings.h"
#include "welcomepageview.h"
#include "digikamitemview.h"
#include "itemiconview.h"
#include "dbinfoiface.h"
#include "itemalbummodel.h"
#include "itemalbumfiltermodel.h"
#include "itempreviewview.h"
#include "itemthumbnailbar.h"
#include "loadingcacheinterface.h"
#include "previewlayout.h"
#include "thumbbardock.h"
#include "tableview.h"
#include "trashview.h"
#include "dimg.h"

#ifdef HAVE_MEDIAPLAYER
#   include "mediaplayerview.h"
#endif // HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE
#   include "mapwidgetview.h"
#endif // HAVE_MARBLE

namespace Digikam
{

class Q_DECL_HIDDEN StackedView::Private
{
public:

    explicit Private()
      : needUpdateBar   (false),
        syncingSelection(false),
        dockArea        (nullptr),
        imageIconView   (nullptr),
        thumbBar        (nullptr),
        imagePreviewView(nullptr),
        thumbBarDock    (nullptr),
        welcomePageView (nullptr),

#ifdef HAVE_MEDIAPLAYER

        mediaPlayerView (nullptr),

#endif // HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE

        mapWidgetView   (nullptr),

#endif // HAVE_MARBLE

        tableView       (nullptr),
        trashView       (nullptr)
    {
    }

    bool              needUpdateBar;
    bool              syncingSelection;

    QMainWindow*      dockArea;

    DigikamItemView*  imageIconView;
    ItemThumbnailBar* thumbBar;
    ItemPreviewView*  imagePreviewView;
    ThumbBarDock*     thumbBarDock;
    WelcomePageView*  welcomePageView;

    QMap<int, int>    stackMap;

#ifdef HAVE_MEDIAPLAYER

    MediaPlayerView*  mediaPlayerView;

#endif // HAVE_MEDIAPLAYER

#ifdef HAVE_MARBLE

    MapWidgetView*    mapWidgetView;

#endif // HAVE_MARBLE

    TableView*        tableView;
    TrashView*        trashView;
};

StackedView::StackedView(QWidget* const parent)
    : QStackedWidget(parent),
      d             (new Private)
{
    d->imageIconView    = new DigikamItemView(this);
    d->imagePreviewView = new ItemPreviewView(this);
    d->thumbBarDock     = new ThumbBarDock();
    d->thumbBar         = new ItemThumbnailBar(d->thumbBarDock);
    d->thumbBar->setModelsFiltered(d->imageIconView->imageModel(),
                                   d->imageIconView->imageFilterModel());
    d->thumbBar->installOverlays();
    d->thumbBarDock->setWidget(d->thumbBar);
    d->thumbBarDock->setObjectName(QLatin1String("mainwindow_thumbbar"));

    d->welcomePageView  = new WelcomePageView(this);

    d->tableView        = new TableView(d->imageIconView->getSelectionModel(),
                                        d->imageIconView->imageFilterModel(),
                                        this);
    d->tableView->setObjectName(QLatin1String("mainwindow_tableview"));

    d->trashView        = new TrashView(this);

#ifdef HAVE_MARBLE

    d->mapWidgetView    = new MapWidgetView(d->imageIconView->getSelectionModel(),
                                            d->imageIconView->imageFilterModel(), this,
                                            MapWidgetView::ApplicationDigikam);
    d->mapWidgetView->setObjectName(QLatin1String("mainwindow_mapwidgetview"));

#endif // HAVE_MARBLE

#ifdef HAVE_MEDIAPLAYER

    d->mediaPlayerView  = new MediaPlayerView(this);
    d->mediaPlayerView->setObjectName(QLatin1String("main_media_player"));
    d->mediaPlayerView->setInfoInterface(new DBInfoIface(this, QList<QUrl>()));

#endif // HAVE_MEDIAPLAYER

    d->stackMap[addWidget(d->imageIconView)]    = IconViewMode;
    d->stackMap[addWidget(d->imagePreviewView)] = PreviewImageMode;
    d->stackMap[addWidget(d->welcomePageView)]  = WelcomePageMode;
    d->stackMap[addWidget(d->tableView)]        = TableViewMode;
    d->stackMap[addWidget(d->trashView)]        = TrashViewMode;

#ifdef HAVE_MARBLE

    d->stackMap[addWidget(d->mapWidgetView)]    = MapWidgetMode;

#endif // HAVE_MARBLE

#ifdef HAVE_MEDIAPLAYER

    d->stackMap[addWidget(d->mediaPlayerView)]  = MediaPlayerMode;

#endif // HAVE_MEDIAPLAYER

    setViewMode(IconViewMode);
    setAttribute(Qt::WA_DeleteOnClose);

    readSettings();

    // -----------------------------------------------------------------

    connect(d->imagePreviewView, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(d->imagePreviewView, SIGNAL(signalGotoAlbumAndItem(ItemInfo)),
            this, SIGNAL(signalGotoAlbumAndItem(ItemInfo)));

    connect(d->imagePreviewView, SIGNAL(signalGotoDateAndItem(ItemInfo)),
            this, SIGNAL(signalGotoDateAndItem(ItemInfo)));

    connect(d->imagePreviewView, SIGNAL(signalGotoTagAndItem(int)),
            this, SIGNAL(signalGotoTagAndItem(int)));

    connect(d->imagePreviewView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->imagePreviewView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->imagePreviewView, SIGNAL(signalDeleteItem()),
            this, SIGNAL(signalDeleteItem()));

    connect(d->imagePreviewView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));

    connect(d->imagePreviewView->layout(), SIGNAL(zoomFactorChanged(double)),
            this, SLOT(slotZoomFactorChanged(double)));

    connect(d->imagePreviewView, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(d->thumbBar, SIGNAL(selectionChanged()),
            this, SLOT(slotThumbBarSelectionChanged()));

    connect(d->imageIconView, SIGNAL(selectionChanged()),
            this, SLOT(slotIconViewSelectionChanged()));

    connect(d->thumbBarDock, SIGNAL(dockLocationChanged(Qt::DockWidgetArea)),
            d->thumbBar, SLOT(slotDockLocationChanged(Qt::DockWidgetArea)));

    connect(d->imagePreviewView, SIGNAL(signalPreviewLoaded(bool)),
            this, SLOT(slotPreviewLoaded(bool)));

#ifdef HAVE_MEDIAPLAYER

    connect(d->mediaPlayerView, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->mediaPlayerView, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));

    connect(d->mediaPlayerView, SIGNAL(signalEscapePreview()),
            this, SIGNAL(signalEscapePreview()));

#endif // HAVE_MEDIAPLAYER

}

StackedView::~StackedView()
{
    delete d;
}

void StackedView::readSettings()
{
    ApplicationSettings* settings = ApplicationSettings::instance();
    bool showThumbbar             = settings->getShowThumbbar();
    d->thumbBarDock->setShouldBeVisible(showThumbbar);
}

void StackedView::setDockArea(QMainWindow* dockArea)
{
    // Attach the thumbbar dock to the given dock area and place it initially on top.

    d->dockArea = dockArea;
    d->thumbBarDock->setParent(d->dockArea);
    d->dockArea->addDockWidget(Qt::TopDockWidgetArea, d->thumbBarDock);
    d->thumbBarDock->setFloating(false);
}

ThumbBarDock* StackedView::thumbBarDock() const
{
    return d->thumbBarDock;
}

ItemThumbnailBar* StackedView::thumbBar() const
{
    return d->thumbBar;
}

void StackedView::slotEscapePreview()
{

#ifdef HAVE_MEDIAPLAYER

    if (viewMode() == MediaPlayerMode)
    {
        d->mediaPlayerView->escapePreview();
    }

#endif //HAVE_MEDIAPLAYER

}

DigikamItemView* StackedView::imageIconView() const
{
    return d->imageIconView;
}

ItemPreviewView* StackedView::imagePreviewView() const
{
    return d->imagePreviewView;
}

#ifdef HAVE_MARBLE

MapWidgetView* StackedView::mapWidgetView() const
{
    return d->mapWidgetView;
}

#endif // HAVE_MARBLE

TableView* StackedView::tableView() const
{
    return d->tableView;
}

TrashView* StackedView::trashView() const
{
    return d->trashView;
}

#ifdef HAVE_MEDIAPLAYER

MediaPlayerView* StackedView::mediaPlayerView() const
{
    return d->mediaPlayerView;
}

#endif // HAVE_MEDIAPLAYER

bool StackedView::isInSingleFileMode() const
{
    return ((viewMode() == PreviewImageMode) ||
            (viewMode() == MediaPlayerMode));
}

bool StackedView::isInMultipleFileMode() const
{
    return ((viewMode() == IconViewMode)  ||
            (viewMode() == MapWidgetMode) ||
            (viewMode() == TableViewMode));
}

bool StackedView::isInAbstractMode() const
{
    return (viewMode() == WelcomePageMode);
}

void StackedView::setPreviewItem(const ItemInfo& info, const ItemInfo& previous, const ItemInfo& next)
{
    if (info.isNull())
    {
        if (viewMode() == MediaPlayerMode)
        {

#ifdef HAVE_MEDIAPLAYER

            d->mediaPlayerView->setCurrentItem();

#endif // HAVE_MEDIAPLAYER

        }
        else if (viewMode() == PreviewImageMode)
        {
            d->imagePreviewView->setItemInfo();
        }
    }
    else
    {
        if ((info.category() == DatabaseItem::Audio)      ||
            (info.category() == DatabaseItem::Video)      ||
            DImg::isAnimatedImage(info.fileUrl().toLocalFile()))    // Special case for animated image as GIF or NMG
        {
            // Stop image viewer

            if (viewMode() == PreviewImageMode)
            {
                d->imagePreviewView->setItemInfo();
            }

#ifdef HAVE_MEDIAPLAYER

            setViewMode(MediaPlayerMode);
            d->mediaPlayerView->setCurrentItem(info.fileUrl(), !previous.isNull(), !next.isNull());

#endif // HAVE_MEDIAPLAYER

        }
        else // Static image or Raw image.
        {
            // Stop media player if running...

            if (viewMode() == MediaPlayerMode)
            {

#ifdef HAVE_MEDIAPLAYER

                d->mediaPlayerView->setCurrentItem();

#endif // HAVE_MEDIAPLAYER

            }

            d->imagePreviewView->setItemInfo(info, previous, next);

            // NOTE: No need to toggle immediately in PreviewImageMode here,
            // because we will receive a signal for that when the image preview will be loaded.
            // This will prevent a flicker effect with the old image preview loaded in stack.
        }

        // do not touch the selection, only adjust current info

        QModelIndex currentIndex = d->thumbBar->imageSortFilterModel()->indexForItemInfo(info);
        d->thumbBar->selectionModel()->setCurrentIndex(currentIndex, QItemSelectionModel::NoUpdate);
    }
}

StackedView::StackedViewMode StackedView::viewMode() const
{
    return StackedViewMode(d->stackMap.value(currentIndex()));
}

void StackedView::setViewMode(const StackedViewMode mode)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Stacked View Mode : " << mode;

    if ((mode < StackedViewModeFirst) || (mode > StackedViewModeLast))
    {
        return;
    }

    if ((mode == PreviewImageMode) || (mode == MediaPlayerMode))
    {
        d->thumbBarDock->restoreVisibility();
        syncSelection(d->imageIconView, d->thumbBar);
    }
    else
    {
        d->thumbBarDock->hide();
    }

    if ((mode == IconViewMode)    ||
        (mode == WelcomePageMode) ||
        (mode == MapWidgetMode)   ||
        (mode == TableViewMode))
    {
        setPreviewItem();
        setCurrentIndex(d->stackMap.key(mode));
    }
    else
    {
        setCurrentIndex(d->stackMap.key(mode));
    }

#ifdef HAVE_MARBLE

    d->mapWidgetView->setActive(mode == MapWidgetMode);

#endif // HAVE_MARBLE

    d->tableView->slotSetActive(mode == TableViewMode);

    if (mode == IconViewMode)
    {
        d->imageIconView->setFocus();
    }

#ifdef HAVE_MARBLE

    else if (mode == MapWidgetMode)
    {
        d->mapWidgetView->setFocus();
    }

#endif // HAVE_MARBLE

    else if (mode == TableViewMode)
    {
        d->tableView->setFocus();
    }

    Q_EMIT signalViewModeChanged();
}

void StackedView::syncSelection(ItemCategorizedView* const from, ItemCategorizedView* const to)
{
    ImageSortFilterModel* const fromModel = from->imageSortFilterModel();
    ImageSortFilterModel* const toModel   = to->imageSortFilterModel();
    QModelIndex currentIndex              = toModel->indexForItemInfo(from->currentInfo());

    // sync selection

    QItemSelection selection              = from->selectionModel()->selection();
    QItemSelection newSelection;

    Q_FOREACH (const QItemSelectionRange& range, selection)
    {
        QModelIndex topLeft     = toModel->indexForItemInfo(fromModel->imageInfo(range.topLeft()));
        QModelIndex bottomRight = toModel->indexForItemInfo(fromModel->imageInfo(range.bottomRight()));
        newSelection.select(topLeft, bottomRight);
    }

    d->syncingSelection = true;

    if (currentIndex.isValid())
    {
        // set current info

        to->setCurrentIndex(currentIndex);
    }

    to->selectionModel()->select(newSelection, QItemSelectionModel::ClearAndSelect);
    d->syncingSelection = false;
}

void StackedView::slotThumbBarSelectionChanged()
{
    if ((viewMode() != PreviewImageMode) && (viewMode() != MediaPlayerMode))
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->thumbBar, d->imageIconView);
}

void StackedView::slotIconViewSelectionChanged()
{
    if (viewMode() != IconViewMode)
    {
        return;
    }

    if (d->syncingSelection)
    {
        return;
    }

    syncSelection(d->imageIconView, d->thumbBar);
}

void StackedView::previewLoaded()
{
    Q_EMIT signalViewModeChanged();
}

void StackedView::slotZoomFactorChanged(double z)
{
    if (viewMode() == PreviewImageMode)
    {
        Q_EMIT signalZoomFactorChanged(z);
    }
}

void StackedView::increaseZoom()
{
    d->imagePreviewView->layout()->increaseZoom();
}

void StackedView::decreaseZoom()
{
    d->imagePreviewView->layout()->decreaseZoom();
}

void StackedView::zoomTo100Percents()
{
    d->imagePreviewView->layout()->setZoomFactor(1.0, QPoint());
}

void StackedView::fitToWindow()
{
    d->imagePreviewView->layout()->fitToWindow();
}

void StackedView::toggleFitToWindowOr100()
{
    d->imagePreviewView->layout()->toggleFitToWindowOr100();
}

bool StackedView::maxZoom()
{
    return d->imagePreviewView->layout()->atMaxZoom();
}

bool StackedView::minZoom()
{
    return d->imagePreviewView->layout()->atMinZoom();
}

void StackedView::setZoomFactor(double z)
{
    // Giving a null anchor means to use the current view center

    d->imagePreviewView->layout()->setZoomFactor(z, QPoint());
}

void StackedView::setZoomFactorSnapped(double z)
{
    d->imagePreviewView->layout()->setZoomFactor(z, QPoint(), SinglePhotoPreviewLayout::SnapZoomFactor);
}

double StackedView::zoomFactor()
{
    return d->imagePreviewView->layout()->zoomFactor();
}

double StackedView::zoomMin()
{
    return d->imagePreviewView->layout()->minZoomFactor();
}

double StackedView::zoomMax()
{
    return d->imagePreviewView->layout()->maxZoomFactor();
}

void StackedView::slotPreviewLoaded(bool)
{
    setViewMode(StackedView::PreviewImageMode);
    previewLoaded();
}

} // namespace Digikam
