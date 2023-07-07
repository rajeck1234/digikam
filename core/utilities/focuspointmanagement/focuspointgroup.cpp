/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Managing of focus point items on a GraphicsDImgView
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "focuspointgroup_p.h"

// Local includes

#include "focuspoints_extractor.h"
#include "focuspoints_writer.h"

namespace Digikam
{

FocusPointGroup::FocusPointGroup(GraphicsDImgView* const view)
    : QObject(view),
      d      (new Private(this))
{
    d->view                 = view;
    d->visibilityController = new ItemVisibilityController(this);
    d->visibilityController->setShallBeShown(false);
/*
    connect(AlbumManager::instance(), SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));
*/
    connect(AlbumManager::instance(), SIGNAL(signalAlbumsUpdated(int)),
            this, SLOT(slotAlbumsUpdated(int)));

    connect(view->previewItem(), SIGNAL(stateChanged(int)),
            this, SLOT(itemStateChanged(int)));
}

FocusPointGroup::~FocusPointGroup()
{
    delete d;
}

void FocusPointGroup::itemStateChanged(int itemState)
{
    switch (itemState)
    {
        case DImgPreviewItem::NoImage:
        case DImgPreviewItem::Loading:
        case DImgPreviewItem::ImageLoadingFailed:
        {
            d->visibilityController->hide();
            break;
        }

        case DImgPreviewItem::ImageLoaded:
        {
            if (d->state == PointsLoaded)
            {
                d->visibilityController->show();
            }

            break;
        }
    }
}

bool FocusPointGroup::isVisible() const
{
    return d->visibilityController->shallBeShown();
}

bool FocusPointGroup::hasVisibleItems() const
{
    return d->visibilityController->hasVisibleItems();
}

ItemInfo FocusPointGroup::info() const
{
    return d->info;
}

QList<RegionFrameItem*> FocusPointGroup::items() const
{
    QList<RegionFrameItem*> items;

    Q_FOREACH (FocusPointItem* const item, d->items)
    {
        items << item;
    }

    return items;
}

void FocusPointGroup::setVisible(bool visible)
{
    d->visibilityController->setShallBeShown(visible);

    if (visible)
    {
        clear();
        load();
    }

    d->applyVisible();
}

void FocusPointGroup::setVisibleItem(RegionFrameItem* const item)
{
    d->visibilityController->setItemThatShallBeShown(item);
    d->applyVisible();
}

void FocusPointGroup::setInfo(const ItemInfo& info)
{
    if ((d->info == info) && (d->state != NoPoints))
    {
        return;
    }

    clear();
    d->info = info;

    if (d->visibilityController->shallBeShown())
    {
        load();
    }
}

void FocusPointGroup::aboutToSetInfoAfterRotate(const ItemInfo& info)
{
    if (d->info == info)
    {
        return;
    }

    clear();
}

void FocusPointGroup::leaveEvent(QEvent*)
{
    if (!isVisible())
    {
        setVisibleItem(nullptr);
    }
}

void FocusPointGroup::load()
{
    if (d->state != NoPoints)
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "FocusPointsGroup: no Point to show";
        return;
    }

    d->state      = LoadingPoints;
    d->exifRotate = (MetaEngineSettings::instance()->settings().exifRotate            ||
                     ((d->view->previewItem()->image().detectedFormat() == DImg::RAW) &&
                      !d->view->previewItem()->image().attribute(QLatin1String("fromRawEmbeddedPreview")).toBool()));

    if (d->info.isNull())
    {
        d->state = PointsLoaded;
        qCDebug(DIGIKAM_GENERAL_LOG) << "FocusPointsGroup: no Point to load";

        return;
    }

    QList<FocusPoint> points = FocusPointsExtractor(this, d->info.filePath()).get_af_points();

    d->visibilityController->clear();

    if (!points.isEmpty())
    {
        d->view->setFocus();
    }

    Q_FOREACH (const auto& point, points)
    {
        d->addItem(point);
    }

    d->state = PointsLoaded;

    if (d->view->previewItem()->isLoaded())
    {
        d->visibilityController->show();
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "FocusPointGroup: points to show:" << points;
}

void FocusPointGroup::clear()
{
    cancelAddItem();
    d->visibilityController->clear();

    Q_FOREACH (RegionFrameItem* const item, d->items)
    {
        delete item;
    }

    d->items.clear();
    d->state = NoPoints;
}

void FocusPointGroup::slotAlbumsUpdated(int type)
{
    if (type != Album::TAG)
    {
        return;
    }

    if (d->items.isEmpty())
    {
        return;
    }

    clear();
    load();
}

/*
void FocusPointGroup::slotAlbumRenamed(Album* const album)
{
    if (!album || (album->type() != Album::TAG))
    {
        return;
    }

    Q_FOREACH (FocusPointItem* const item, d->items)
    {
        if (!item->point().isNull() &&
            (item->point().tagId() == album->id()))
        {
            item->updateCurrentTag();
        }
    }
}
*/

void FocusPointGroup::addPoint()
{
    if (d->manuallyAddWrapItem)
    {
        return;
    }

    setVisible(false);
    d->manuallyAddWrapItem = new ClickDragReleaseItem(d->view->previewItem());
    d->manuallyAddWrapItem->setFocus();
    d->view->setFocus();

    connect(d->manuallyAddWrapItem, SIGNAL(started(QPointF)),
            this, SLOT(slotAddItemStarted(QPointF)));

    connect(d->manuallyAddWrapItem, SIGNAL(moving(QRectF)),
            this, SLOT(slotAddItemMoving(QRectF)));

    connect(d->manuallyAddWrapItem, SIGNAL(finished(QRectF)),
            this, SLOT(slotAddItemFinished(QRectF)));

    connect(d->manuallyAddWrapItem, SIGNAL(cancelled()),
            this, SLOT(cancelAddItem()));
}

void FocusPointGroup::slotAddItemStarted(const QPointF& pos)
{
    Q_UNUSED(pos);
}

void FocusPointGroup::slotAddItemMoving(const QRectF& rect)
{
    d->visibilityController->setItemThatShallBeShown(d->manuallyAddedItem);

    if (!d->manuallyAddedItem)
    {
        d->manuallyAddedItem = d->createItem(FocusPoint());
        d->visibilityController->addItem(d->manuallyAddedItem);
        d->visibilityController->showItem(d->manuallyAddedItem);
    }

    d->manuallyAddedItem->setRectInSceneCoordinatesAdjusted(rect);
}

void FocusPointGroup::slotAddItemFinished(const QRectF& rect)
{
    if (d->manuallyAddedItem)
    {
        d->manuallyAddedItem->setRectInSceneCoordinatesAdjusted(rect);
        QRect pointRect            = d->manuallyAddedItem->originalRect();
        DImg preview(d->view->previewItem()->image().copy());

        if (!d->exifRotate)
        {
            TagRegion::adjustToOrientation(pointRect,
                                           d->info.orientation(),
                                           d->info.dimensions());
            preview.rotateAndFlip(d->info.orientation());
        }

        QRectF pointRectF = TagRegion::absoluteToRelative(pointRect, d->info.dimensions());

        QScopedPointer<FocusPointsWriter> writer(new FocusPointsWriter(this, d->info.filePath()));
        writer->writeFocusPoint(pointRectF);

        FocusPoint point(pointRectF);
        point.setType(FocusPoint::TypePoint::SelectedInFocus);
        FocusPointItem* const item = d->addItem(point);

        d->visibilityController->setItemDirectlyVisible(item, true);
        d->manuallyAddWrapItem->stackBefore(item);
    }

    cancelAddItem();
}

void FocusPointGroup::cancelAddItem()
{
    delete d->manuallyAddedItem;
    d->manuallyAddedItem = nullptr;

    if (d->manuallyAddWrapItem)
    {
        d->view->scene()->removeItem(d->manuallyAddWrapItem);
        d->manuallyAddWrapItem->deleteLater();
        d->manuallyAddWrapItem = nullptr;
    }
}

bool FocusPointGroup::isAllowedToAddFocusPoint() const
{
    FocusPointsExtractor extractor(nullptr, d->info.filePath());

    return (!extractor.isAFPointsReadOnly());
}

} // namespace Digikam
