/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-07-20
 * Description : Loader for thumbnails - Freedesktop.org standard implementation
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2003-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "thumbnailcreator_p.h"

namespace Digikam
{

ThumbnailInfo ThumbnailCreator::fileThumbnailInfo(const QString& path)
{
    ThumbnailInfo info;

    if (path.isEmpty())
    {
        return info;
    }

    info.filePath     = path;
    QFileInfo fileInfo(path);
    info.isAccessible = fileInfo.exists();
    info.fileName     = fileInfo.fileName();
    QString suffix    = fileInfo.suffix().toUpper();

    QMimeDatabase mimeDB;
    QString mimeType(mimeDB.mimeTypeForFile(path).name());

    if      (mimeType.startsWith(QLatin1String("image/")) ||
             (suffix == QLatin1String("PGF"))             ||
             (suffix == QLatin1String("JXL"))             ||
             (suffix == QLatin1String("AVIF"))            ||
             (suffix == QLatin1String("KRA"))             ||
             (suffix == QLatin1String("CR3"))             ||
             (suffix == QLatin1String("HIF"))             ||
             (suffix == QLatin1String("HEIC"))            ||
             (suffix == QLatin1String("HEIF")))
    {
        info.mimeType = QLatin1String("image");
    }
    else if (mimeType.startsWith(QLatin1String("video/")))
    {
        info.mimeType = QLatin1String("video");
    }

    if (!info.isAccessible)
    {
        return info;
    }

    info.modificationDate = fileInfo.lastModified();

    return info;
}

ThumbnailImage ThumbnailCreator::loadFreedesktop(const ThumbnailInfo& info) const
{
    QString path;

    if (!info.customIdentifier.isNull())
    {
        path = info.customIdentifier;
    }
    else
    {
        path = info.filePath;
    }

    QString uri       = thumbnailUri(path);
    QString thumbPath = thumbnailPath(path);
    QImage qimage     = loadPNG(thumbPath);

    // NOTE: if thumbnail have not been generated by digiKam (konqueror for example),
    // force to recompute it, else we use it.

    if (!qimage.isNull())
    {
        if ((qimage.text(QLatin1String("Thumb::MTime")) == QString::number(info.modificationDate.toSecsSinceEpoch())) &&
            (qimage.text(QLatin1String("Software"))     == d->digiKamFingerPrint))
        {
            ThumbnailImage imgInfo;
            imgInfo.qimage = qimage;

            // is stored rotated. Not needed to rotate.

            imgInfo.exifOrientation = DMetadata::ORIENTATION_NORMAL;

            return imgInfo;
        }
    }

    return ThumbnailImage();
}

void ThumbnailCreator::storeFreedesktop(const ThumbnailInfo& info, const ThumbnailImage& image) const
{
    QImage qimage = image.qimage;

    QString path;

    if (!info.customIdentifier.isNull())
    {
        path = info.customIdentifier;
    }
    else
    {
        path = info.filePath;
    }

    QString uri       = thumbnailUri(path);
    QString thumbPath = thumbnailPath(path);

    // required by spec

    if (qimage.format() != QImage::Format_ARGB32)
    {
        qimage = qimage.convertToFormat(QImage::Format_ARGB32);
    }

    qimage.setText(QLatin1String("Thumb::URI"),   uri);
    qimage.setText(QLatin1String("Thumb::MTime"), QString::number(info.modificationDate.toSecsSinceEpoch()));
    qimage.setText(QLatin1String("Software"),     d->digiKamFingerPrint);

    QTemporaryFile temp;
    temp.setFileTemplate(thumbPath + QLatin1String("-digikam-") + QLatin1String("XXXXXX") + QLatin1String(".png"));
    temp.setAutoRemove(false);

    if (temp.open())
    {
        QString tempFileName = temp.fileName();

        if (qimage.save(tempFileName, "PNG", 0))
        {
            Q_ASSERT(!tempFileName.isEmpty());

            temp.close();

            // remove thumbPath file if it exist

            if ((tempFileName != thumbPath) && QFile::exists(tempFileName) && QFile::exists(thumbPath))
            {
                QFile::remove(thumbPath);
            }

            if (!QFile::rename(tempFileName, thumbPath))
            {
                qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot rename thumb file (" << tempFileName << ")";
                qCDebug(DIGIKAM_GENERAL_LOG) << "to (" << thumbPath << ")...";
            }
        }
    }
}

void ThumbnailCreator::deleteFromDiskFreedesktop(const QString& filePath) const
{
    QFile smallThumb(thumbnailPath(filePath, normalThumbnailDir()));
    QFile largeThumb(thumbnailPath(filePath, largeThumbnailDir()));

    smallThumb.remove();
    largeThumb.remove();
}

} // namespace Digikam
