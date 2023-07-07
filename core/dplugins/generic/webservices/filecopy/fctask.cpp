/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-03-27
 * Description : file copy actions using threads.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fctask.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "dfileoperations.h"
#include "previewloadthread.h"
#include "dmetadata.h"
#include "dimg.h"

namespace DigikamGenericFileCopyPlugin
{

class Q_DECL_HIDDEN FCTask::Private
{
public:

    explicit Private()
    {
    }

    QUrl        srcUrl;
    FCContainer settings;
};

FCTask::FCTask(const QUrl& srcUrl,
               const FCContainer& settings)
    : ActionJob(),
      d        (new Private)
{
    d->srcUrl   = srcUrl;
    d->settings = settings;
}

FCTask::~FCTask()
{
    cancel();
    delete d;
}

void FCTask::run()
{
    if (m_cancel)
    {
        return;
    }

    bool ok   = true;
    QUrl dest = d->settings.destUrl.adjusted(QUrl::StripTrailingSlash);

    if (d->settings.iface && d->settings.iface->supportAlbums() && d->settings.albumPath)
    {
        DInfoInterface::DInfoMap infoMap  = d->settings.iface->itemInfo(d->srcUrl);
        DItemInfo info(infoMap);
        DInfoInterface::DInfoMap albumMap = d->settings.iface->albumInfo(info.albumId());
        DAlbumInfo album(albumMap);

        dest.setPath(dest.path() + album.albumPath());
        dest = dest.adjusted(QUrl::StripTrailingSlash);

        if (!QDir(dest.toLocalFile()).exists())
        {
            ok = QDir().mkpath(dest.toLocalFile());
        }
    }

    dest.setPath(dest.path() +
                 QLatin1Char('/') +
                 d->srcUrl.fileName());

    if (d->srcUrl == dest)
    {
        Q_EMIT signalDone();

        return;
    }

    QUrl sidecarDest = DMetadata::sidecarUrl(dest);

    if      (ok && (d->settings.behavior == FCContainer::CopyFile))
    {
        QFileInfo srcInfo(d->srcUrl.toLocalFile());
        QString suffix = srcInfo.suffix().toUpper();

        QMimeDatabase mimeDB;
        QString mimeType(mimeDB.mimeTypeForUrl(d->srcUrl).name());

        if (d->settings.changeImageProperties             &&
            (
             mimeType.startsWith(QLatin1String("image/")) ||
             (suffix == QLatin1String("PGF"))             ||
             (suffix == QLatin1String("JXL"))             ||
             (suffix == QLatin1String("AVIF"))            ||
             (suffix == QLatin1String("KRA"))             ||
             (suffix == QLatin1String("HIF"))             ||
             (suffix == QLatin1String("HEIC"))            ||
             (suffix == QLatin1String("HEIF"))
            )
           )
        {
            ok = imageResize(d->srcUrl.toLocalFile(), dest);
        }
        else
        {
            dest = getUrlOrDelete(dest);
            ok   = DFileOperations::copyFile(d->srcUrl.toLocalFile(),
                                             dest.toLocalFile());

            if (d->settings.sidecars && DMetadata::hasSidecar(d->srcUrl.toLocalFile()))
            {
                sidecarDest = getUrlOrDelete(sidecarDest);
                DFileOperations::copyFile(DMetadata::sidecarUrl(d->srcUrl).toLocalFile(),
                                          sidecarDest.toLocalFile());
            }
        }
    }
    else if (ok                                                     &&
             ((d->settings.behavior == FCContainer::FullSymLink)    ||
              (d->settings.behavior == FCContainer::RelativeSymLink)))
    {

#ifdef Q_OS_WIN

        dest.setPath(dest.path() + QLatin1String(".lnk"));
        sidecarDest.setPath(sidecarDest.path() + QLatin1String(".lnk"));

#endif

        dest        = getUrlOrDelete(dest);
        sidecarDest = getUrlOrDelete(sidecarDest);

        if (d->settings.behavior == FCContainer::FullSymLink)
        {
            ok = QFile::link(d->srcUrl.toLocalFile(),
                             dest.toLocalFile());

            if (d->settings.sidecars && DMetadata::hasSidecar(d->srcUrl.toLocalFile()))
            {
                QFile::link(DMetadata::sidecarUrl(d->srcUrl).toLocalFile(),
                            sidecarDest.toLocalFile());
            }
        }
        else
        {
            QDir dir(d->settings.destUrl.toLocalFile());
            QString path = dir.relativeFilePath(d->srcUrl.toLocalFile());
            QUrl srcUrl  = QUrl::fromLocalFile(path);
            ok           = QFile::link(srcUrl.toLocalFile(),
                                       dest.toLocalFile());

            if (d->settings.sidecars && DMetadata::hasSidecar(d->srcUrl.toLocalFile()))
            {
                QFile::link(DMetadata::sidecarUrl(srcUrl).toLocalFile(),
                            sidecarDest.toLocalFile());
            }
        }
    }

    if (ok)
    {
        Q_EMIT signalUrlProcessed(d->srcUrl, dest);
    }

    Q_EMIT signalDone();
}

bool FCTask::imageResize(const QString& orgPath, QUrl& destUrl)
{
    QFileInfo fi(orgPath);

    if (!fi.exists() || !fi.isReadable())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error opening input file"
                                         << fi.filePath();
        return false;
    }

    QFileInfo destInfo(destUrl.toLocalFile());
    QFileInfo tmpDir(destInfo.dir().absolutePath());

    if (!tmpDir.exists() || !tmpDir.isWritable())
    {
        qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Error opening target folder"
                                         << tmpDir.dir();
        return false;
    }

    DImg img = PreviewLoadThread::loadHighQualitySynchronously(orgPath);

    if (img.isNull())
    {
        img.load(orgPath);
    }

    if (!img.isNull())
    {
        uint sizeFactor = d->settings.imageResize;

        if ((img.width() > sizeFactor) || (img.height() > sizeFactor))
        {
            DImg scaledImg = img.smoothScale(sizeFactor,
                                             sizeFactor,
                                             Qt::KeepAspectRatio);

            if ((scaledImg.width() > sizeFactor) || (scaledImg.height() > sizeFactor))
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cannot resize image";
                return false;
            }

            img = scaledImg;
        }

        QString destFile = destInfo.path()  +
                           QLatin1Char('/') +
                           destInfo.completeBaseName();

        if (d->settings.imageFormat == FCContainer::JPEG)
        {
            destFile.append(QLatin1String(".jpg"));
            destUrl  = getUrlOrDelete(QUrl::fromLocalFile(destFile));
            destFile = destUrl.toLocalFile();

            if (d->settings.sidecars)
            {
                getUrlOrDelete(DMetadata::sidecarUrl(destUrl));
            }

            img.setAttribute(QLatin1String("quality"), d->settings.imageCompression);

            if (!img.save(destFile, DImg::JPEG))
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cannot save resized image (JPEG)";
                return false;
            }
        }
        else if (d->settings.imageFormat == FCContainer::PNG)
        {
            destFile.append(QLatin1String(".png"));
            destUrl  = getUrlOrDelete(QUrl::fromLocalFile(destFile));
            destFile = destUrl.toLocalFile();

            if (d->settings.sidecars)
            {
                getUrlOrDelete(DMetadata::sidecarUrl(destUrl));
            }

            if (!img.save(destFile, DImg::PNG))
            {
                qCDebug(DIGIKAM_WEBSERVICES_LOG) << "Cannot save resized image (PNG)";
                return false;
            }
        }

        QScopedPointer<DMetadata> meta(new DMetadata);

        if (!meta->load(destFile))
        {
            return false;
        }

        if (d->settings.removeMetadata)
        {
            meta->clearExif();
            meta->clearIptc();
            meta->clearXmp();
        }
        else
        {
            meta->setItemOrientation(MetaEngine::ORIENTATION_NORMAL);
        }

        if (!meta->save(destFile))
        {
            return false;
        }

        DFileOperations::copyModificationTime(orgPath, destFile);

        return true;
    }

    return false;
}

QUrl FCTask::getUrlOrDelete(const QUrl& fileUrl) const
{
    if (d->settings.overwrite              &&
        QFile::exists(fileUrl.toLocalFile()))
    {
        QFile::remove(fileUrl.toLocalFile());

        return fileUrl;
    }

    return DFileOperations::getUniqueFileUrl(fileUrl);
}

} // namespace DigikamGenericFileCopyPlugin
