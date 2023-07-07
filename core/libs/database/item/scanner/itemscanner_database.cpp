/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning a single item - database helper.
 *
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemscanner_p.h"

namespace Digikam
{

qlonglong ItemScanner::id() const
{
    return d->scanInfo.id;
}

void ItemScanner::commit()
{
    qCDebug(DIGIKAM_DATABASE_LOG) << "Scanning took" << d->timer.restart() << "ms";

    switch (d->commit.operation)
    {
        case ItemScannerCommit::NoOp:
        {
            return;
        }

        case ItemScannerCommit::AddItem:
        {
            if (!commitAddImage())
            {
                return;
            }

            break;
        }

        case ItemScannerCommit::UpdateItem:
        {
            commitUpdateImage();
            break;
        }
    }

    if (d->commit.copyImageAttributesId != -1)
    {
        commitCopyImageAttributes();
        return;
    }

    if (d->commit.commitItemInformation)
    {
        commitItemInformation();
    }

    if (d->commit.commitImageMetadata)
    {
        commitImageMetadata();
    }
    else if (d->commit.commitVideoMetadata)
    {
        commitVideoMetadata();
    }

    if (d->commit.commitItemPosition)
    {
        commitItemPosition();
    }

    if (d->commit.commitItemComments)
    {
        commitItemComments();
    }

    if (d->commit.commitItemCopyright)
    {
        commitItemCopyright();
    }

    if (d->commit.commitIPTCCore)
    {
        commitIPTCCore();
    }

    if (!d->commit.tagIds.isEmpty())
    {
        commitTags();
    }

    if (d->commit.commitFaces)
    {
        commitFaces();
    }

    commitImageHistory();
}

void ItemScanner::newFile(int albumId)
{
    loadFromDisk();
    prepareAddImage(albumId);

    if (!scanFromIdenticalFile())
    {
        scanFile(NewScan);
    }
}

void ItemScanner::newFileFullScan(int albumId)
{
    loadFromDisk();
    prepareAddImage(albumId);
    scanFile(NewScan);
}

void ItemScanner::rescan()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(Rescan);
}

void ItemScanner::cleanScan()
{
    loadFromDisk();
    prepareUpdateImage();
    scanFile(CleanScan);
}

void ItemScanner::copiedFrom(int albumId, qlonglong srcId)
{
    loadFromDisk();
    prepareAddImage(albumId);

    // first use source, if it exists

    if (!copyFromSource(srcId))
    {
        // check if we can establish identity

        if (!scanFromIdenticalFile())
        {
            // scan newly

            scanFile(NewScan);
        }
    }
}

void ItemScanner::commitCopyImageAttributes()
{
    CoreDbAccess().db()->copyImageAttributes(d->commit.copyImageAttributesId, d->scanInfo.id);

    // Also copy the similarity information

    SimilarityDbAccess().db()->copySimilarityAttributes(d->commit.copyImageAttributesId, d->scanInfo.id);

    // Remove grouping for copied or identical images.

    CoreDbAccess().db()->removeAllImageRelationsFrom(d->scanInfo.id, DatabaseRelation::Grouped);
    CoreDbAccess().db()->removeAllImageRelationsTo(d->scanInfo.id, DatabaseRelation::Grouped);
}

bool ItemScanner::copyFromSource(qlonglong srcId)
{
    // some basic validity checking

    if (srcId == d->scanInfo.id)
    {
        return false;
    }

    ItemScanInfo info = CoreDbAccess().db()->getItemScanInfo(srcId);

    if (!info.id)
    {
        return false;
    }

    qCDebug(DIGIKAM_DATABASE_LOG) << "Recognized" << d->fileInfo.filePath() << "as copied from" << srcId;
    d->commit.copyImageAttributesId = srcId;

    return true;
}

void ItemScanner::prepareAddImage(int albumId)
{
    d->scanInfo.albumID = albumId;
    d->scanInfo.status  = DatabaseItem::Visible;

    qCDebug(DIGIKAM_DATABASE_LOG) << "Adding new item" << d->fileInfo.filePath();
    d->commit.operation = ItemScannerCommit::AddItem;
}

bool ItemScanner::commitAddImage()
{
    // find the image id of a deleted image info if existent and mark it as valid.
    // otherwise, create a new item.

    qlonglong imageId = CoreDbAccess().db()->findImageId(-1, d->scanInfo.itemName, DatabaseItem::Status::Trashed,
                                                         d->scanInfo.category, d->scanInfo.fileSize, d->scanInfo.uniqueHash);

    if (imageId != -1 && (d->commit.copyImageAttributesId == -1))
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Detected identical image info with id" << imageId
                                      << "and album id NULL of a removed image for image" << d->scanInfo.itemName;
        qCDebug(DIGIKAM_DATABASE_LOG) << "Will reuse this image info and set the status to visible and the album id to" << d->scanInfo.albumID;

        d->scanInfo.id = imageId;
        CoreDbAccess().db()->setItemAlbum(imageId, d->scanInfo.albumID);
        CoreDbAccess().db()->setItemStatus(imageId, DatabaseItem::Status::Visible);

        return false;
    }
    else
    {
        d->scanInfo.id = CoreDbAccess().db()->addItem(d->scanInfo.albumID, d->scanInfo.itemName,
                                                      d->scanInfo.status, d->scanInfo.category,
                                                      d->scanInfo.modificationDate, d->scanInfo.fileSize,
                                                      d->scanInfo.uniqueHash);
    }

    return true;
}

void ItemScanner::cleanDatabaseMetadata()
{
    if (d->scanInfo.id == -1)
    {
        return;
    }

    const MetaEngineSettingsContainer& settings = MetaEngineSettings::instance()->settings();
    QList<int> removeTags;

    if (settings.saveColorLabel)
    {
        const QVector<int>& colorTags = TagsCache::instance()->colorLabelTags();
        removeTags << QList<int>(colorTags.begin(), colorTags.end());
    }

    if (settings.savePickLabel)
    {
        const QVector<int>& pickTags = TagsCache::instance()->pickLabelTags();
        removeTags << QList<int>(pickTags.begin(), pickTags.end());
    }

    if (settings.saveTags)
    {
        Q_FOREACH (int tag, CoreDbAccess().db()->getItemTagIDs(d->scanInfo.id))
        {
            if (!TagsCache::instance()->isInternalTag(tag))
            {
                removeTags << tag;
            }
        }
    }

    if (!removeTags.isEmpty())
    {
        CoreDbAccess().db()->removeTagsFromItems(QList<qlonglong>() << d->scanInfo.id, removeTags);
    }

    if (settings.savePosition)
    {
        CoreDbAccess().db()->removeItemPosition(d->scanInfo.id);
    }

    if (settings.saveTemplate)
    {
        CoreDbAccess().db()->removeAllImageProperties(d->scanInfo.id);
        CoreDbAccess().db()->removeAllItemCopyrightProperties(d->scanInfo.id);
    }

    if (settings.saveComments)
    {
        CoreDbAccess().db()->removeAllImageComments(d->scanInfo.id);
    }

    if (settings.saveFaceTags)
    {
        Q_FOREACH (const ImageTagProperty& property, CoreDbAccess().db()->getImageTagProperties(d->scanInfo.id))
        {
            if (property.property == ImageTagPropertyName::tagRegion())
            {
                CoreDbAccess().db()->removeImageTagProperties(d->scanInfo.id, property.tagId, property.property);
            }
        }
    }

    if (settings.saveRating)
    {
        CoreDbAccess().db()->changeItemInformation(d->scanInfo.id, QVariantList() << NoRating, DatabaseFields::Rating);
    }
}

} // namespace Digikam
