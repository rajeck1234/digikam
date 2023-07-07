/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust thread.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustthread.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "dmetadata.h"
#include "dinfointerface.h"
#include "timeadjusttask.h"

namespace DigikamGenericTimeAdjustPlugin
{

class Q_DECL_HIDDEN TimeAdjustThread::Private
{

public:

    explicit Private()
      : iface(nullptr)
    {
    }

    // Settings from GUI.

    TimeAdjustContainer settings;

    QMap<QUrl, int>     itemsMap;

    DInfoInterface*     iface;
};


TimeAdjustThread::TimeAdjustThread(QObject* const parent, DInfoInterface* const iface)
    : ActionThreadBase(parent),
      d(new Private)
{
    d->iface = iface;
}

TimeAdjustThread::~TimeAdjustThread()
{
    // cancel the thread

    cancel();

    // wait for the thread to finish

    wait();

    delete d;
}

void TimeAdjustThread::setUpdatedDates(const QMap<QUrl, int>& itemsMap)
{
    d->itemsMap = itemsMap;

    ActionJobCollection collection;

    Q_FOREACH (const QUrl& url, itemsMap.keys())
    {
        TimeAdjustTask* const t = new TimeAdjustTask(url, this);
        t->setSettings(d->settings);

        connect(t, SIGNAL(signalProcessStarted(QUrl)),
                this, SIGNAL(signalProcessStarted(QUrl)));

        connect(t, SIGNAL(signalProcessEnded(QUrl,QDateTime,QDateTime,int)),
                this, SIGNAL(signalProcessEnded(QUrl,QDateTime,QDateTime,int)));

        connect(t, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)),
                this, SIGNAL(signalDateTimeForUrl(QUrl,QDateTime,bool)));

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void TimeAdjustThread::setPreviewDates(const QMap<QUrl, int>& itemsMap)
{
    d->itemsMap = itemsMap;

    ActionJobCollection collection;

    Q_FOREACH (const QUrl& url, itemsMap.keys())
    {
        TimePreviewTask* const t = new TimePreviewTask(url, this);
        t->setSettings(d->settings);

        connect(t, SIGNAL(signalPreviewReady(QUrl,QDateTime,QDateTime)),
                this, SIGNAL(signalPreviewReady(QUrl,QDateTime,QDateTime)));

        collection.insert(t, 0);
     }

    appendJobs(collection);
}

void TimeAdjustThread::setSettings(const TimeAdjustContainer& settings)
{
    d->settings = settings;
}

QDateTime TimeAdjustThread::readTimestamp(const QUrl& url) const
{
    QDateTime dateTime;

    switch (d->settings.dateSource)
    {
        case TimeAdjustContainer::APPDATE:
        {
            dateTime = readApplicationTimestamp(url);
            break;
        }

        case TimeAdjustContainer::FILENAME:
        {
            dateTime = readFileNameTimestamp(url);
            break;
        }

        case TimeAdjustContainer::FILEDATE:
        {
            dateTime = readFileTimestamp(url);
            break;
        }

        case TimeAdjustContainer::METADATADATE:
        {
            dateTime = readMetadataTimestamp(url);
            break;
        }

        default:  // CUSTOMDATE
        {
            dateTime = QDateTime(d->settings.customDate.date(),
                                 d->settings.customTime.time());

            break;
        }
    }

    return dateTime;
}

QDateTime TimeAdjustThread::readApplicationTimestamp(const QUrl& url) const
{
    DItemInfo info(d->iface->itemInfo(url));

    if (info.dateTime().isValid())
    {
        return info.dateTime();
    }

    return QDateTime();
}

QDateTime TimeAdjustThread::readFileNameTimestamp(const QUrl& url) const
{
    return d->settings.getDateTimeFromString(url.fileName());
}

QDateTime TimeAdjustThread::readFileTimestamp(const QUrl& url) const
{
    QFileInfo fileInfo(url.toLocalFile());

    return fileInfo.lastModified();
}

QDateTime TimeAdjustThread::readMetadataTimestamp(const QUrl& url) const
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (!meta->load(url.toLocalFile()))
    {
        return QDateTime();
    }

    QDateTime dateTime;
    QString exifDateTimeFormat = QLatin1String("yyyy:MM:dd hh:mm:ss");
    QString xmpDateTimeFormat  = QLatin1String("yyyy-MM-ddThh:mm:ss");

    switch (d->settings.metadataSource)
    {
        case TimeAdjustContainer::EXIFIPTCXMP:
        {
            dateTime = meta->getItemDateTime();
            break;
        }

        case TimeAdjustContainer::EXIFCREATED:
        {
            dateTime = QDateTime::fromString(meta->getExifTagString("Exif.Image.DateTime"),
                                             exifDateTimeFormat);
            break;
        }

        case TimeAdjustContainer::EXIFORIGINAL:
        {
            dateTime = QDateTime::fromString(meta->getExifTagString("Exif.Photo.DateTimeOriginal"),
                                             exifDateTimeFormat);
            break;
        }

        case TimeAdjustContainer::EXIFDIGITIZED:
        {
            dateTime = QDateTime::fromString(meta->getExifTagString("Exif.Photo.DateTimeDigitized"),
                                             exifDateTimeFormat);
            break;
        }

        case TimeAdjustContainer::IPTCCREATED:
        {
            // we have to truncate the timezone from the time, otherwise it cannot be converted to a QTime

            dateTime = QDateTime(QDate::fromString(meta->getIptcTagString("Iptc.Application2.DateCreated"),
                                                   Qt::ISODate),
                                 QTime::fromString(meta->getIptcTagString("Iptc.Application2.TimeCreated").left(8),
                                                   Qt::ISODate));
            break;
        }

        case TimeAdjustContainer::XMPCREATED:
        {
            dateTime = QDateTime::fromString(meta->getXmpTagString("Xmp.xmp.CreateDate"),
                                             xmpDateTimeFormat);
            break;
        }

        case TimeAdjustContainer::FUZZYCREATED:
        {
            dateTime = d->settings.getDateTimeFromString(meta->getExifTagString("Exif.Image.DateTime"));
            break;
        }

        case TimeAdjustContainer::FUZZYORIGINAL:
        {
            dateTime = d->settings.getDateTimeFromString(meta->getExifTagString("Exif.Photo.DateTimeOriginal"));
            break;
        }

        case TimeAdjustContainer::FUZZYDIGITIZED:
        {
            dateTime = d->settings.getDateTimeFromString(meta->getExifTagString("Exif.Photo.DateTimeDigitized"));
            break;
        }

        default:
        {
            // dateTime stays invalid

            break;
        }
    };

    return dateTime;
}

int TimeAdjustThread::indexForUrl(const QUrl& url) const
{
    return d->itemsMap.value(url);
}

} // namespace DigikamGenericTimeAdjustPlugin
