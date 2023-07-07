/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-31
 * Description : time adjust actions using threads.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjusttask.h"

// Qt includes

#include <QFile>
#include <QScopedPointer>
#include <qplatformdefs.h>

// Local includes

#include "dmetadata.h"
#include "digikam_debug.h"
#include "dinfointerface.h"
#include "dfileoperations.h"
#include "timeadjustlist.h"
#include "timeadjustthread.h"
#include "metaenginesettings.h"

namespace DigikamGenericTimeAdjustPlugin
{

class Q_DECL_HIDDEN TimeAdjustTask::Private
{
public:

    explicit Private()
      : thread(nullptr)
    {
    }

    // Settings from GUI.

    TimeAdjustContainer   settings;

    TimeAdjustThread*     thread;

    QUrl                  url;
};

TimeAdjustTask::TimeAdjustTask(const QUrl& url, TimeAdjustThread* const thread)
    : ActionJob(),
      d(new Private)
{
    d->url    = url;
    d->thread = thread;
}

TimeAdjustTask::~TimeAdjustTask()
{
    cancel();
    delete d;
}

void TimeAdjustTask::setSettings(const TimeAdjustContainer& settings)
{
    d->settings = settings;
}

void TimeAdjustTask::run()
{
    if (m_cancel)
    {
        return;
    }

    Q_EMIT signalProcessStarted(d->url);

    QDateTime org = d->thread->readTimestamp(d->url);
    QDateTime adj = d->settings.calculateAdjustedDate(org, d->thread->indexForUrl(d->url));

    if (!adj.isValid())
    {
        Q_EMIT signalProcessEnded(d->url, org, adj, TimeAdjustList::META_TIME_ERROR);
        Q_EMIT signalDone();
        return;
    }

    if (m_cancel)
    {
        return;
    }

    bool metadataChanged               = false;
    bool writeToSidecar                = (MetaEngineSettings::instance()->settings()
                                          .metadataWritingMode != DMetadata::WRITE_TO_FILE_ONLY);
    bool writeWithExifTool             = (MetaEngineSettings::instance()->settings().writeWithExifTool);

    int status                         = TimeAdjustList::NOPROCESS_ERROR;

    QString exifDateTimeFormat         = QLatin1String("yyyy:MM:dd hh:mm:ss");
    QString xmpDateTimeFormat          = QLatin1String("yyyy-MM-ddThh:mm:ss");

    const QMap<QString, bool>& tagsMap = d->settings.getDateTimeTagsMap();
    QMap<QString, bool>::const_iterator it;

    QScopedPointer<DMetadata> meta(new DMetadata);

    if (meta->load(d->url.toLocalFile()))
    {
        for (it = tagsMap.constBegin() ; it != tagsMap.constEnd() ; ++it)
        {
            if (!it.value())
            {
                continue;
            }

            bool ret = true;

            if      (it.key().startsWith(QLatin1String("Exif.")) &&
                     (meta->canWriteExif(d->url.toLocalFile())   ||
                      writeWithExifTool                          ||
                      writeToSidecar)
                    )
            {
                if (!d->settings.updIfAvailable ||
                    !meta->getExifTagString(it.key().toLatin1().constData()).isEmpty())
                {
                    ret &= meta->setExifTagString(it.key().toLatin1().constData(),
                                                  adj.toString(exifDateTimeFormat));

                    metadataChanged = true;
                }
            }
            else if (it.key().startsWith(QLatin1String("Iptc.")) &&
                     (meta->canWriteIptc(d->url.toLocalFile())   ||
                      writeWithExifTool                          ||
                      writeToSidecar)
                    )
            {
                if (!d->settings.updIfAvailable ||
                    !meta->getIptcTagString(it.key().toLatin1().constData()).isEmpty())
                {
                    if      (it.key().contains(QLatin1String("Date")))
                    {
                        ret &= meta->setIptcTagString(it.key().toLatin1().constData(),
                                                      adj.date().toString(Qt::ISODate));

                        metadataChanged = true;
                    }
                    else if (it.key().contains(QLatin1String("Time")))
                    {
                        ret &= meta->setIptcTagString(it.key().toLatin1().constData(),
                                                      adj.time().toString(Qt::ISODate));

                        metadataChanged = true;
                    }
                }
            }
            else if (it.key().startsWith(QLatin1String("Xmp.")) &&
                     (meta->canWriteXmp(d->url.toLocalFile())   ||
                      writeWithExifTool                         ||
                      writeToSidecar)
                     )
            {
                if (!d->settings.updIfAvailable ||
                    !meta->getXmpTagString(it.key().toLatin1().constData()).isEmpty())
                {
                    ret &= meta->setXmpTagString(it.key().toLatin1().constData(),
                                                 adj.toString(xmpDateTimeFormat));

                    metadataChanged = true;
                }
            }

            if (!ret)
            {
                qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Failed to set metadata for tag" << it.key();

                status |= TimeAdjustList::META_TIME_ERROR;

                break;
            }
        }

        if ((status == TimeAdjustList::NOPROCESS_ERROR) && metadataChanged)
        {
            if (!meta->save(d->url.toLocalFile()))
            {
                qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Failed to update metadata in file" << d->url.fileName();

                status |= TimeAdjustList::META_TIME_ERROR;
            }
        }
    }
    else
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Failed to load metadata from file" << d->url.fileName();

        status |= TimeAdjustList::META_TIME_ERROR;
    }

    if (d->settings.updFileModDate)
    {
        if (!DFileOperations::setModificationTime(d->url.toLocalFile(), adj))
        {
            status |= TimeAdjustList::FILE_TIME_ERROR;
        }
    }

    if (writeToSidecar && DMetadata::hasSidecar(d->url.toLocalFile()))
    {
        if (!DFileOperations::copyModificationTime(d->url.toLocalFile(), DMetadata::sidecarPath(d->url.toLocalFile())))
        {
            status |= TimeAdjustList::FILE_TIME_ERROR;
        }
    }

    if ((status & TimeAdjustList::META_TIME_ERROR) != TimeAdjustList::META_TIME_ERROR)
    {
        Q_EMIT signalDateTimeForUrl(d->url, adj, d->settings.updFileModDate);
    }

    Q_EMIT signalProcessEnded(d->url, org, adj, status);
    Q_EMIT signalDone();
}

// ------------------------------------------------------------------

class Q_DECL_HIDDEN TimePreviewTask::Private
{
public:

    explicit Private()
      : thread(nullptr)
    {
    }

    // Settings from GUI.

    TimeAdjustContainer   settings;

    TimeAdjustThread*     thread;

    QUrl                  url;
};

TimePreviewTask::TimePreviewTask(const QUrl& url, TimeAdjustThread* const thread)
    : ActionJob(),
      d(new Private)
{
    d->url    = url;
    d->thread = thread;
}

TimePreviewTask::~TimePreviewTask()
{
    cancel();
    delete d;
}

void TimePreviewTask::setSettings(const TimeAdjustContainer& settings)
{
    d->settings = settings;
}

void TimePreviewTask::run()
{
    if (m_cancel)
    {
        return;
    }

    QDateTime org = d->thread->readTimestamp(d->url);
    QDateTime adj = d->settings.calculateAdjustedDate(org, d->thread->indexForUrl(d->url));

    Q_EMIT signalPreviewReady(d->url, org, adj);
    Q_EMIT signalDone();
}

} // namespace DigikamGenericTimeAdjustPlugin
