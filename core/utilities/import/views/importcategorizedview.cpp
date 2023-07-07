/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-07-13
 * Description : Qt categorized item view for camera items
 *
 * SPDX-FileCopyrightText: 2012 by Islam Wazery <wazery at ubuntu dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importcategorizedview.h"

// Qt includes

#include <QTimer>

// Local includes

#include "digikam_debug.h"
#include "camitemsortsettings.h"
#include "iccsettings.h"
#include "itemselectionoverlay.h"
#include "importdelegate.h"
#include "importtooltipfiller.h"
#include "importsettings.h"
#include "itemviewtooltip.h"
#include "loadingcacheinterface.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class Q_DECL_HIDDEN ImportItemViewToolTip : public ItemViewToolTip
{
    Q_OBJECT

public:

    explicit ImportItemViewToolTip(ImportCategorizedView* const view)
        : ItemViewToolTip(view)
    {
    }

    ImportCategorizedView* view() const
    {
        return static_cast<ImportCategorizedView*>(ItemViewToolTip::view());
    }

protected:

    QString tipContents() override
    {
        CamItemInfo info = ImportItemModel::retrieveCamItemInfo(currentIndex());

        return ImportToolTipFiller::CamItemInfoTipContents(info);
    }
};

class Q_DECL_HIDDEN ImportCategorizedView::Private
{
public:

    explicit Private()
      : model            (nullptr),
        filterModel      (nullptr),
        delegate         (nullptr),
        showToolTip      (false),
        scrollToItemId   (0),
        delayedEnterTimer(nullptr),
        currentMouseEvent(nullptr)
    {
    }

    ImportItemModel*       model;
    ImportSortFilterModel* filterModel;

    ImportDelegate*        delegate;
    bool                   showToolTip;

    qlonglong              scrollToItemId;

    QTimer*                delayedEnterTimer;

    QMouseEvent*           currentMouseEvent;
};

ImportCategorizedView::ImportCategorizedView(QWidget* const parent)
    : ItemViewCategorized(parent),
      d                  (new Private)
{
    setToolTip(new ImportItemViewToolTip(this));

    LoadingCacheInterface::connectToSignalFileChanged(this,
            SLOT(slotFileChanged(QString)));

    d->delayedEnterTimer = new QTimer(this);
    d->delayedEnterTimer->setInterval(10);
    d->delayedEnterTimer->setSingleShot(true);

    connect(d->delayedEnterTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedEnter()));

    connect(IccSettings::instance(), SIGNAL(signalICCSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)),
            this, SLOT(slotIccSettingsChanged(ICCSettingsContainer,ICCSettingsContainer)));
}

ImportCategorizedView::~ImportCategorizedView()
{
    d->delegate->removeAllOverlays();
    delete d;
}

void ImportCategorizedView::setModels(ImportItemModel* model, ImportSortFilterModel* filterModel)
{
    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(false);
    }

    if (d->filterModel)
    {
        disconnect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
                   this, SLOT(layoutAboutToBeChanged()));

        disconnect(d->filterModel, SIGNAL(layoutChanged()),
                   this, SLOT(layoutWasChanged()));
    }

    if (d->model)
    {
        disconnect(d->model, SIGNAL(itemInfosAdded(QList<CamItemInfo>)),
                   this, SLOT(slotCamItemInfosAdded()));
    }

    d->model       = model;
    d->filterModel = filterModel;

    setModel(d->filterModel);

    connect(d->filterModel, SIGNAL(layoutAboutToBeChanged()),
            this, SLOT(layoutAboutToBeChanged()));

    connect(d->filterModel, SIGNAL(layoutChanged()),
            this, SLOT(layoutWasChanged()),
            Qt::QueuedConnection);

    connect(d->model, SIGNAL(itemInfosAdded(QList<CamItemInfo>)),
            this, SLOT(slotCamItemInfosAdded()));

    Q_EMIT modelChanged();

    if (d->delegate)
    {
        d->delegate->setAllOverlaysActive(true);
    }
}

ImportItemModel* ImportCategorizedView::importItemModel() const
{
    return d->model;
}

ImportSortFilterModel* ImportCategorizedView::importSortFilterModel() const
{
    return d->filterModel;
}

ImportFilterModel* ImportCategorizedView::importFilterModel() const
{
    return d->filterModel->importFilterModel();
}

ImportThumbnailModel* ImportCategorizedView::importThumbnailModel() const
{
    return qobject_cast<ImportThumbnailModel*>(d->model);
}

QSortFilterProxyModel* ImportCategorizedView::filterModel() const
{
    return d->filterModel;
}

ImportDelegate* ImportCategorizedView::delegate() const
{
    return d->delegate;
}

void ImportCategorizedView::setItemDelegate(ImportDelegate* delegate)
{
    ThumbnailSize oldSize       = thumbnailSize();
    ImportDelegate* oldDelegate = d->delegate;

    if (oldDelegate)
    {
        hideIndexNotification();
        d->delegate->setAllOverlaysActive(false);
        d->delegate->setViewOnAllOverlays(nullptr);

        // Note: Be precise, no wildcard disconnect!

        disconnect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
                   this, SLOT(showIndexNotification(QModelIndex,QString)));

        disconnect(d->delegate, SIGNAL(hideNotification()),
                   this, SLOT(hideIndexNotification()));
    }

    d->delegate = delegate;
    delegate->setThumbnailSize(oldSize);

    if (oldDelegate)
    {
        d->delegate->setSpacing(oldDelegate->spacing());
    }

    ItemViewCategorized::setItemDelegate(d->delegate);
    setCategoryDrawer(d->delegate->categoryDrawer());
    updateDelegateSizes();

    d->delegate->setViewOnAllOverlays(this);
    d->delegate->setAllOverlaysActive(true);

    connect(d->delegate, SIGNAL(requestNotification(QModelIndex,QString)),
            this, SLOT(showIndexNotification(QModelIndex,QString)));

    connect(d->delegate, SIGNAL(hideNotification()),
            this, SLOT(hideIndexNotification()));
}

CamItemInfo ImportCategorizedView::currentInfo() const
{
    return d->filterModel->camItemInfo(currentIndex());
}

QUrl ImportCategorizedView::currentUrl() const
{
    return currentInfo().url();
}

QList<CamItemInfo> ImportCategorizedView::selectedCamItemInfos() const
{
    return d->filterModel->camItemInfos(selectedIndexes());
}

QList<CamItemInfo> ImportCategorizedView::selectedCamItemInfosCurrentFirst() const
{
    QList<QModelIndex> indexes = selectedIndexes();
    QModelIndex        current = currentIndex();
    QList<CamItemInfo> infos;

    Q_FOREACH (const QModelIndex& index, indexes)
    {
        CamItemInfo info = d->filterModel->camItemInfo(index);

        if (index == current)
        {
            infos.prepend(info);
        }
        else
        {
            infos.append(info);
        }
    }

    return infos;
}

QList<CamItemInfo> ImportCategorizedView::camItemInfos() const
{
    return d->filterModel->camItemInfosSorted();
}

QList<QUrl> ImportCategorizedView::urls() const
{
    QList<CamItemInfo> infos = camItemInfos();
    QList<QUrl>        urls;

    Q_FOREACH (const CamItemInfo& info, infos)
    {
        urls << info.url();
    }

    return urls;
}

QList<QUrl> ImportCategorizedView::selectedUrls() const
{
    QList<CamItemInfo> infos = selectedCamItemInfos();
    QList<QUrl>        urls;

    Q_FOREACH (const CamItemInfo& info, infos)
    {
        urls << info.url();
    }

    return urls;
}

void ImportCategorizedView::toIndex(const QUrl& url)
{
    ItemViewCategorized::toIndex(d->filterModel->indexForPath(url.toLocalFile()));
}

CamItemInfo ImportCategorizedView::nextInOrder(const CamItemInfo& startingPoint, int nth)
{
    QModelIndex index = d->filterModel->indexForCamItemInfo(startingPoint);

    if (!index.isValid())
    {
        return CamItemInfo();
    }

    return d->filterModel->camItemInfo(d->filterModel->index(index.row() + nth, 0, QModelIndex()));
}

QModelIndex ImportCategorizedView::nextIndexHint(const QModelIndex& anchor, const QItemSelectionRange& removed) const
{
    QModelIndex hint = ItemViewCategorized::nextIndexHint(anchor, removed);
    CamItemInfo info = d->filterModel->camItemInfo(anchor);
/*
    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Having initial hint" << hint << "for" << anchor << d->model->numberOfIndexesForCamItemInfo(info);
*/
    // Fixes a special case of multiple (face) entries for the same image.
    // If one is removed, any entry of the same image shall be preferred.

    if (d->model->numberOfIndexesForCamItemInfo(info) > 1)
    {
        // The hint is for a different info, but we may have a hint for the same info

        if (info != d->filterModel->camItemInfo(hint))
        {
            int minDiff                              = d->filterModel->rowCount();
            QList<QModelIndex> indexesForCamItemInfo = d->filterModel->mapListFromSource(d->model->indexesForCamItemInfo(info));

            Q_FOREACH (const QModelIndex& index, indexesForCamItemInfo)
            {
                if ((index == anchor) || !index.isValid() || removed.contains(index))
                {
                    continue;
                }

                int distance = qAbs(index.row() - anchor.row());

                if (distance < minDiff)
                {
                    minDiff = distance;
                    hint    = index;
/*
                    qCDebug(DIGIKAM_IMPORTUI_LOG) << "Chose index" << hint << "at distance" << minDiff << "to" << anchor;
*/
                }
            }
        }
    }

    return hint;
}

ThumbnailSize ImportCategorizedView::thumbnailSize() const
{
/*
    ImportThumbnailModel *thumbModel = importThumbnailModel();

    if (thumbModel)
    {
        return thumbModel->thumbnailSize();
    }
*/
    if (d->delegate)
    {
        return d->delegate->thumbnailSize();
    }

    return ThumbnailSize();
}

void ImportCategorizedView::setThumbnailSize(int size)
{
    setThumbnailSize(ThumbnailSize(size));
}

void ImportCategorizedView::setThumbnailSize(const ThumbnailSize& s)
{
    // we abuse this pair of method calls to restore scroll position
    // TODO check if needed

    layoutAboutToBeChanged();
    d->delegate->setThumbnailSize(s);
    layoutWasChanged();
}

void ImportCategorizedView::setCurrentWhenAvailable(qlonglong camItemId)
{
    d->scrollToItemId = camItemId;
}

void ImportCategorizedView::setCurrentUrl(const QUrl& url)
{
    if (url.isEmpty())
    {
        clearSelection();
        setCurrentIndex(QModelIndex());
        return;
    }

    QString path      = url.toLocalFile();
    QModelIndex index = d->filterModel->indexForPath(path);

    if (!index.isValid())
    {
        return;
    }

    clearSelection();
    setCurrentIndex(index);
}

void ImportCategorizedView::setCurrentInfo(const CamItemInfo& info)
{
    QModelIndex index = d->filterModel->indexForCamItemInfo(info);
    clearSelection();
    setCurrentIndex(index);
}

void ImportCategorizedView::setSelectedUrls(const QList<QUrl>& urlList)
{
    QItemSelection mySelection;

    for (QList<QUrl>::const_iterator it = urlList.constBegin() ; it!=urlList.constEnd() ; ++it)
    {
        const QString path      = it->toLocalFile();
        const QModelIndex index = d->filterModel->indexForPath(path);

        if (!index.isValid())
        {
            qCWarning(DIGIKAM_IMPORTUI_LOG) << "no QModelIndex found for" << *it;
        }
        else
        {
            // TODO: is there a better way?

            mySelection.select(index, index);
        }
    }

    clearSelection();
    selectionModel()->select(mySelection, QItemSelectionModel::Select);
}

void ImportCategorizedView::setSelectedCamItemInfos(const QList<CamItemInfo>& infos)
{
    QItemSelection mySelection;

    Q_FOREACH (const CamItemInfo& info, infos)
    {
        QModelIndex index = d->filterModel->indexForCamItemInfo(info);
        mySelection.select(index, index);
    }

    selectionModel()->select(mySelection, QItemSelectionModel::ClearAndSelect);
}

void ImportCategorizedView::hintAt(const CamItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    QModelIndex index = d->filterModel->indexForCamItemInfo(info);

    if (!index.isValid())
    {
        return;
    }

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
    scrollTo(index);
}

void ImportCategorizedView::addOverlay(ItemDelegateOverlay* overlay, ImportDelegate* delegate)
{
    if (!delegate)
    {
        delegate = d->delegate;
    }

    delegate->installOverlay(overlay);

    if (delegate == d->delegate)
    {
        overlay->setView(this);
        overlay->setActive(true);
    }
}

void ImportCategorizedView::removeOverlay(ItemDelegateOverlay* overlay)
{
    ImportDelegate* delegate = dynamic_cast<ImportDelegate*>(overlay->delegate());

    if (delegate)
    {
        delegate->removeOverlay(overlay);
    }

    overlay->setView(nullptr);
}

void ImportCategorizedView::updateGeometries()
{
    ItemViewCategorized::updateGeometries();
    d->delayedEnterTimer->start();
}

void ImportCategorizedView::slotDelayedEnter()
{
    // re-Q_EMIT entered() for index under mouse (after layout).

    QModelIndex mouseIndex = indexAt(mapFromGlobal(QCursor::pos()));

    if (mouseIndex.isValid())
    {
        Q_EMIT DCategorizedView::entered(mouseIndex);
    }
}

void ImportCategorizedView::addSelectionOverlay(ImportDelegate* delegate)
{
    addOverlay(new ItemSelectionOverlay(this), delegate);
}

void ImportCategorizedView::scrollToStoredItem()
{
    if (d->scrollToItemId)
    {
        if (d->model->hasImage(d->scrollToItemId))
        {
            QModelIndex index = d->filterModel->indexForCamItemId(d->scrollToItemId);
            setCurrentIndex(index);
            scrollToRelaxed(index, QAbstractItemView::PositionAtCenter);
            d->scrollToItemId = 0;
        }
    }
}

void ImportCategorizedView::slotCamItemInfosAdded()
{
    if (d->scrollToItemId)
    {
        scrollToStoredItem();
    }
}

void ImportCategorizedView::slotFileChanged(const QString& filePath)
{
    QModelIndex index = d->filterModel->indexForPath(filePath);

    if (index.isValid())
    {
        update(index);
    }
}

void ImportCategorizedView::indexActivated(const QModelIndex& index, Qt::KeyboardModifiers modifiers)
{
    CamItemInfo info = d->filterModel->camItemInfo(index);

    if (!info.isNull())
    {
        activated(info, modifiers);
        Q_EMIT camItemInfoActivated(info);
    }
}

void ImportCategorizedView::currentChanged(const QModelIndex& index, const QModelIndex& previous)
{
    ItemViewCategorized::currentChanged(index, previous);

    Q_EMIT currentChanged(d->filterModel->camItemInfo(index));
}

void ImportCategorizedView::selectionChanged(const QItemSelection& selectedItems, const QItemSelection& deselectedItems)
{
    ItemViewCategorized::selectionChanged(selectedItems, deselectedItems);

    if (!selectedItems.isEmpty())
    {
        Q_EMIT selected(d->filterModel->camItemInfos(selectedItems.indexes()));
    }

    if (!deselectedItems.isEmpty())
    {
        Q_EMIT deselected(d->filterModel->camItemInfos(deselectedItems.indexes()));
    }
}

void ImportCategorizedView::activated(const CamItemInfo&, Qt::KeyboardModifiers)
{
    // implemented in subclass
}

void ImportCategorizedView::showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index)
{
    CamItemInfo info = d->filterModel->camItemInfo(index);
    showContextMenuOnInfo(event, info);
}

void ImportCategorizedView::showContextMenuOnInfo(QContextMenuEvent*, const CamItemInfo&)
{
    // implemented in subclass
}

void ImportCategorizedView::paintEvent(QPaintEvent* e)
{
    ItemViewCategorized::paintEvent(e);
}

QItemSelectionModel* ImportCategorizedView::getSelectionModel() const
{
    return selectionModel();
}

AbstractItemDragDropHandler* ImportCategorizedView::dragDropHandler() const
{
    return d->model->dragDropHandler();
}

void ImportCategorizedView::slotIccSettingsChanged(const ICCSettingsContainer&, const ICCSettingsContainer&)
{
    viewport()->update();
}

} // namespace Digikam

#include "importcategorizedview.moc"
