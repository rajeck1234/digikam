/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "metadatasynchronizer.h"

// Qt includes

#include <QString>
#include <QTimer>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "albummanager.h"
#include "iteminfojob.h"
#include "maintenancethread.h"

namespace Digikam
{

class Q_DECL_HIDDEN MetadataSynchronizer::Private
{
public:

    explicit Private()
      : imageInfoJob(nullptr),
        thread(nullptr),
        direction(MetadataSynchronizer::WriteFromDatabaseToFile),
        tagsOnly(false)
    {
    }

    AlbumList                           palbumList;
    AlbumList::Iterator                 albumsIt;

    ItemInfoJob*                        imageInfoJob;

    ItemInfoList                        imageInfoList;

    MaintenanceThread*                  thread;

    MetadataSynchronizer::SyncDirection direction;
    bool                                tagsOnly;
};

MetadataSynchronizer::MetadataSynchronizer(const AlbumList& list, SyncDirection direction, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("MetadataSynchronizer"), parent),
      d(new Private)
{
    if (list.isEmpty())
        d->palbumList = AlbumManager::instance()->allPAlbums();
    else
        d->palbumList = list;

    init(direction);
}

MetadataSynchronizer::MetadataSynchronizer(const ItemInfoList& list, SyncDirection direction, ProgressItem* const parent)
    : MaintenanceTool(QLatin1String("MetadataSynchronizer"), parent),
      d(new Private)
{
    d->imageInfoList = list;
    init(direction);
}

// Common methods ----------------------------------------------------------------------------

void MetadataSynchronizer::setTagsOnly(bool value)
{
    d->tagsOnly = value;
}

void MetadataSynchronizer::init(SyncDirection direction)
{
    d->direction = direction;
    d->thread    = new MaintenanceThread(this);

    connect(d->thread, SIGNAL(signalCompleted()),
            this, SLOT(slotDone()));

    connect(d->thread, SIGNAL(signalAdvance(QImage)),
            this, SLOT(slotAdvance()));
}

void MetadataSynchronizer::setUseMultiCoreCPU(bool b)
{
    d->thread->setUseMultiCore(b);
}

void MetadataSynchronizer::slotStart()
{
    MaintenanceTool::slotStart();

    ProgressManager::addProgressItem(this);

    d->imageInfoJob = new ItemInfoJob;

    connect(d->imageInfoJob, SIGNAL(signalItemsInfo(ItemInfoList)),
            this, SLOT(slotAlbumParsed(ItemInfoList)));

    connect(d->imageInfoJob, SIGNAL(signalCompleted()),
            this, SLOT(slotOneAlbumIsComplete()));

    connect(this, SIGNAL(progressItemCanceled(ProgressItem*)),
            this, SLOT(slotCancel()));

    QTimer::singleShot(500, this, SLOT(slotParseAlbums()));
}

MetadataSynchronizer::~MetadataSynchronizer()
{
    delete d->imageInfoJob;
    delete d;
}

void MetadataSynchronizer::slotCancel()
{
    d->imageInfoJob->stop();
    d->thread->cancel();
    MaintenanceTool::slotCancel();
}

// Parse Albums methods ------------------------------------------------------------------

void MetadataSynchronizer::slotParseAlbums()
{
    setUsesBusyIndicator(true);
    d->albumsIt = d->palbumList.begin();
    processOneAlbum();
}

void MetadataSynchronizer::processOneAlbum()
{
    if (canceled())
    {
        return;
    }

    if (d->albumsIt == d->palbumList.end())     // All albums are parsed.
    {
        parseList();
        return;
    }

    d->imageInfoJob->allItemsFromAlbum(*d->albumsIt);
}

void MetadataSynchronizer::slotAlbumParsed(const ItemInfoList& list)
{
    d->imageInfoList << list;
}

void MetadataSynchronizer::slotOneAlbumIsComplete()
{
    d->albumsIt++;
    processOneAlbum();
}

// Parse info list methods -----------------------------------------------------------------------

void MetadataSynchronizer::parseList()
{
    setUsesBusyIndicator(false);

    if (d->direction == WriteFromDatabaseToFile)
    {
        setLabel(i18n("Synchronizing image metadata with database"));
        setThumbnail(QIcon::fromTheme(QLatin1String("document-edit")).pixmap(22));
    }
    else
    {
        setLabel(i18n("Updating database from image metadata"));
        setThumbnail(QIcon::fromTheme(QLatin1String("edit-redo")).pixmap(22));
    }

    if (d->imageInfoList.isEmpty())
    {
        slotDone();
        return;
    }

    setTotalItems(d->imageInfoList.count());

    d->thread->syncMetadata(d->imageInfoList, d->direction, d->tagsOnly);
    d->thread->start();
}

void MetadataSynchronizer::slotAdvance()
{
    advance(1);
}

} // namespace Digikam
