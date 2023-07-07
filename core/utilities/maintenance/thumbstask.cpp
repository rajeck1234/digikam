/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for thumbs generator.
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbstask.h"

// Qt includes
#include <QMutex>
#include <QWaitCondition>

// Local includes

#include "digikam_debug.h"
#include "thumbnailloadthread.h"
#include "thumbnailsize.h"
#include "maintenancedata.h"

namespace Digikam
{

class Q_DECL_HIDDEN ThumbsTask::Private
{
public:

    explicit Private()
        : thread (nullptr),
          data   (nullptr)
    {
    }

    ThumbnailLoadThread* thread;

    MaintenanceData*     data;

    QImage               image;

    QMutex               mutex;
    QWaitCondition       condVar;
};

// -------------------------------------------------------

ThumbsTask::ThumbsTask()
    : ActionJob(),
      d        (new Private)
{
    d->thread = new ThumbnailLoadThread;
    d->thread->setPixmapRequested(false);
    d->thread->setThumbnailSize(ThumbnailLoadThread::maximumThumbnailSize());

    connect(d->thread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QImage)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QImage)),
            Qt::DirectConnection);
}

ThumbsTask::~ThumbsTask()
{
    cancel();

    {
        QMutexLocker locker(&d->mutex);
        d->condVar.wakeAll();
    }

    d->thread->stopAllTasks();
    d->thread->wait();

    delete d->thread;
    delete d;
}

void ThumbsTask::setMaintenanceData(MaintenanceData* const data)
{
    d->data = data;
}

void ThumbsTask::run()
{
    // While we have data (using this as check for non-null)

    while (d->data)
    {
        if (m_cancel)
        {
            return;
        }

        QString path = d->data->getImagePath();

        if (path.isEmpty())
        {
            break;
        }

        ItemInfo info = ItemInfo::fromLocalFile(path);

        if (!m_cancel && !info.isNull())
        {
            d->thread->deleteThumbnail(info.filePath());
            d->thread->find(info.thumbnailIdentifier());

            if (!m_cancel && d->image.isNull())
            {
                QMutexLocker locker(&d->mutex);
                d->condVar.wait(&d->mutex);
            }

            Q_EMIT signalFinished(d->image);
            d->image = QImage();
        }
    }

    Q_EMIT signalDone();
}

void ThumbsTask::slotThumbnailLoaded(const LoadingDescription&, const QImage& image)
{
    QMutexLocker locker(&d->mutex);

    d->image = image;
    d->condVar.wakeAll();
}

} // namespace Digikam
