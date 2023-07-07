/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - history helpers.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

bool DMetadata::setItemHistory(const QString& imageHistoryXml) const
{
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.ImageHistory", imageHistoryXml))
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    return false;
}

QString DMetadata::getItemHistory() const
{
    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.ImageHistory", false);
/*
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Loading image history " << value;
*/
        return value;
    }

    return QString();
}

bool DMetadata::hasItemHistoryTag() const
{
    if (hasXmp())
    {
        if (QString(getXmpTagString("Xmp.digiKam.ImageHistory", false)).length() > 0)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}

QString DMetadata::getItemUniqueId() const
{
    QString exifUid;

    if (hasXmp())
    {
        QString uuid = getXmpTagString("Xmp.digiKam.ImageUniqueID");

        if (!uuid.isEmpty())
        {
            return uuid;
        }

        exifUid = getXmpTagString("Xmp.exif.ImageUniqueId");
    }

    if (exifUid.isEmpty())
    {
        exifUid = getExifTagString("Exif.Photo.ImageUniqueID");
    }

    // same makers may choose to use a "click counter" to generate the id,
    // which is then weak and not a universally unique id
    // The Exif ImageUniqueID is 128bit, or 32 hex digits.
    // If the first 20 are zero, it's probably a counter,
    // the left 12 are sufficient for more then 10^14 clicks.

    if (!exifUid.isEmpty()                                         &&
        !exifUid.startsWith(QLatin1String("00000000000000000000")) &&
        !getExifTagString("Exif.Image.Make").contains(QLatin1String("SAMSUNG"), Qt::CaseInsensitive))
    {
        return exifUid;
    }

    // Exif.Image.ImageID can also be a pathname, so it's not sufficiently unique

    QString dngUid = getExifTagString("Exif.Image.RawDataUniqueID");

    if (!dngUid.isEmpty())
    {
        return dngUid;
    }

    return QString();
}

bool DMetadata::setItemUniqueId(const QString& uuid) const
{
    if (supportXmp())
    {
        return setXmpTagString("Xmp.digiKam.ImageUniqueID", uuid);
    }

    return false;
}

} // namespace Digikam
