/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-23
 * Description : item metadata interface - labels helpers.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2011      by Leif Huhn <leif at dkstat dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dmetadata.h"

// Qt includes

#include <QLocale>

// Local includes

#include "metaenginesettings.h"
#include "digikam_version.h"
#include "digikam_globals.h"
#include "digikam_debug.h"

namespace Digikam
{

int DMetadata::getItemPickLabel() const
{
    if (hasXmp())
    {
        QString value = getXmpTagString("Xmp.digiKam.PickLabel", false);

        if (!value.isEmpty())
        {
            bool ok     = false;
            long pickId = value.toLong(&ok);

            if (ok && (pickId >= NoPickLabel) && (pickId <= AcceptedLabel))
            {
                return pickId;
            }
        }
    }

    return -1;
}

int DMetadata::getItemColorLabel(const DMetadataSettingsContainer& settings) const
{
    bool xmpSupported  = hasXmp();
    bool exivSupported = hasExif();

    Q_FOREACH (const NamespaceEntry& entry, settings.getReadMapping(NamespaceEntry::DM_COLORLABEL_CONTAINER()))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();
        QString value;

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (xmpSupported)
                {
                    value = getXmpTagString(nameSpace, false);
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (exivSupported)
                {
                    value = getExifTagString(nameSpace, false);
                }

                break;
            }

            default:
            {
                break;
            }
        }

        if (value.isEmpty())
        {
            continue;
        }

        bool ok      = false;
        long colorId = value.toLong(&ok);

        if (ok && (colorId >= NoColorLabel) && (colorId <= WhiteLabel))
        {
            return colorId;
        }

        // LightRoom use this tag to store color name as string.
        // Values are limited : see bug #358193.

        if      (value == QLatin1String("Blue"))
        {
            return BlueLabel;
        }
        else if (value == QLatin1String("Green"))
        {
            return GreenLabel;
        }
        else if (value == QLatin1String("Red"))
        {
            return RedLabel;
        }
        else if (value == QLatin1String("Yellow"))
        {
            return YellowLabel;
        }
        else if (value == QLatin1String("Purple"))
        {
            return MagentaLabel;
        }
    }

    return -1;
}

int DMetadata::getItemRating(const DMetadataSettingsContainer& settings) const
{
    long rating        = -1;
    bool xmpSupported  = hasXmp();
    bool iptcSupported = hasIptc();
    bool exivSupported = hasExif();

    Q_FOREACH (const NamespaceEntry& entry, settings.getReadMapping(NamespaceEntry::DM_RATING_CONTAINER()))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();
        QString value;

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (xmpSupported)
                {
                    value = getXmpTagString(nameSpace, false);
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                if (iptcSupported)
                {
                    value = QString::fromUtf8(getIptcTagData(nameSpace));
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (exivSupported)
                {
                    if (!getExifTagLong(nameSpace, rating))
                    {
                        continue;
                    }
                }

                break;
            }

            default:
            {
                break;
            }
        }

        if (!value.isEmpty())
        {
            bool ok = false;
            rating  = value.toLong(&ok);

            if (!ok)
            {
                return -1;
            }
        }

        int index = entry.convertRatio.indexOf(rating);

        // Exact value was not found,but rating is in range,
        // so we try to approximate it

        if ((index == -1)                         &&
            (rating > entry.convertRatio.first()) &&
            (rating < entry.convertRatio.last()))
        {
            for (int i = 0 ; i < entry.convertRatio.size() ; ++i)
            {
                if (rating > entry.convertRatio.at(i))
                {
                    index = i;
                }
            }
        }

        if (index != -1)
        {
            return index;
        }
    }

    return -1;
}

bool DMetadata::setItemPickLabel(int pickId) const
{
    if ((pickId < NoPickLabel) || (pickId > AcceptedLabel))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Pick Label value to write is out of range!";
        return false;
    }
/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Pick Label: " << pickId;
*/
    if (supportXmp())
    {
        if (!setXmpTagString("Xmp.digiKam.PickLabel", QString::number(pickId)))
        {
            return false;
        }
    }

    return true;
}

bool DMetadata::setItemColorLabel(int colorId, const DMetadataSettingsContainer& settings) const
{
    if ((colorId < NoColorLabel) || (colorId > WhiteLabel))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Color Label value to write is out of range!";

        return false;
    }
/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Color Label: " << colorId;
*/
    QList<NamespaceEntry> toWrite = settings.getReadMapping(NamespaceEntry::DM_COLORLABEL_CONTAINER());

    if (!settings.unifyReadWrite())
    {
        toWrite = settings.getWriteMapping(NamespaceEntry::DM_COLORLABEL_CONTAINER());
    }

    for (const NamespaceEntry& entry : qAsConst(toWrite))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (!supportXmp())
                {
                    continue;
                }

                if (QLatin1String(nameSpace) == QLatin1String("Xmp.xmp.Label"))
                {
                    // LightRoom use this XMP tags to store Color Labels name
                    // Values are limited : see bug #358193.

                    QString LRLabel;

                    switch (colorId)
                    {
                        case BlueLabel:
                        {
                            LRLabel = QLatin1String("Blue");
                            break;
                        }

                        case GreenLabel:
                        {
                            LRLabel = QLatin1String("Green");
                            break;
                        }

                        case RedLabel:
                        {
                            LRLabel = QLatin1String("Red");
                            break;
                        }

                        case YellowLabel:
                        {
                            LRLabel = QLatin1String("Yellow");
                            break;
                        }

                        case MagentaLabel:
                        {
                            LRLabel = QLatin1String("Purple");
                            break;
                        }
                    }

                    if (!LRLabel.isEmpty())
                    {
                        if (!setXmpTagString(nameSpace, LRLabel))
                        {
                            return false;
                        }
                    }
                    else
                    {
                        removeXmpTag(nameSpace);
                    }
                }
                else
                {
                    if (!setXmpTagString(nameSpace, QString::number(colorId)))
                    {
                        return false;
                    }
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (!setExifTagString(nameSpace, QString::number(colorId)))
                {
                    return false;
                }

                break;
            }

            case NamespaceEntry::IPTC:
            {
                break;
            }

            default:
            {
                break;
            }
        }
    }

    return true;
}

bool DMetadata::setItemRating(int rating, const DMetadataSettingsContainer& settings) const
{
    // NOTE : with digiKam 0.9.x, we have used IPTC Urgency to store Rating.
    // Now this way is obsolete, and we use standard XMP rating tag instead.

    if ((rating < RatingMin) || (rating > RatingMax))
    {
        qCDebug(DIGIKAM_METAENGINE_LOG) << "Rating value to write is out of range!";
        return false;
    }
/*
    qCDebug(DIGIKAM_METAENGINE_LOG) << getFilePath() << " ==> Rating:" << rating;
*/
    QList<NamespaceEntry> toWrite = settings.getReadMapping(NamespaceEntry::DM_RATING_CONTAINER());

    if (!settings.unifyReadWrite())
    {
        toWrite = settings.getWriteMapping(NamespaceEntry::DM_RATING_CONTAINER());
    }

    for (const NamespaceEntry& entry : qAsConst(toWrite))
    {
        if (entry.isDisabled)
        {
            continue;
        }

        const std::string myStr = entry.namespaceName.toStdString();
        const char* nameSpace   = myStr.data();

        switch (entry.subspace)
        {
            case NamespaceEntry::XMP:
            {
                if (!supportXmp())
                {
                    continue;
                }

                if (!setXmpTagString(nameSpace, QString::number(entry.convertRatio.at(rating))))
                {
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting rating failed" << nameSpace;
                    return false;
                }

                break;
            }

            case NamespaceEntry::EXIF:
            {
                if (!setExifTagLong(nameSpace, entry.convertRatio.at(rating)))
                {
                    qCDebug(DIGIKAM_METAENGINE_LOG) << "Setting rating failed" << nameSpace;
                    return false;
                }

                break;
            }

            case NamespaceEntry::IPTC: // IPTC rating deprecated
            default:
            {
                break;
            }
        }
    }

    return true;
}

} // namespace Digikam
