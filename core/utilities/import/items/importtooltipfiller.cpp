/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-28-07
 * Description : Import icon view tool tip
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "importtooltipfiller.h"

// Qt includes

#include <QDateTime>
#include <QTextDocument>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "importsettings.h"
#include "itempropertiestab.h"
#include "ditemtooltip.h"
#include "camiteminfo.h"

namespace Digikam
{

QString ImportToolTipFiller::CamItemInfoTipContents(const CamItemInfo& info)
{
    QString str;
    ImportSettings* const settings = ImportSettings::instance();
    DToolTipStyleSheet cnt(settings->getToolTipsFont());

    PhotoInfoContainer photoInfo   = info.photoInfo;
    QString tip                    = cnt.tipHeader;

    // -- File properties ----------------------------------------------

    if (settings->getToolTipsShowFileName()  ||
        settings->getToolTipsShowFileDate()  ||
        settings->getToolTipsShowFileSize()  ||
        settings->getToolTipsShowImageType() ||
        settings->getToolTipsShowImageDim())
    {
        tip += cnt.headBeg + i18n("File Properties") + cnt.headEnd;

        if (settings->getToolTipsShowFileName())
        {
            tip += cnt.cellBeg + i18nc("filename",
                                       "Name:") + cnt.cellMid;
            tip += info.name + cnt.cellEnd;
        }

        if (settings->getToolTipsShowFileDate())
        {
            QDateTime createdDate  = info.ctime;
            str                    = QLocale().toString(createdDate, QLocale::ShortFormat);
            tip                   += cnt.cellBeg + i18n("Date:") + cnt.cellMid + str + cnt.cellEnd;
        }

        if (settings->getToolTipsShowFileSize())
        {
            tip                   += cnt.cellBeg + i18n("Size:") + cnt.cellMid;
            QString localeFileSize = QLocale().toString(info.size);
            str                    = i18n("%1 (%2)", ItemPropertiesTab::humanReadableBytesCount(info.size), localeFileSize);
            tip                   += str + cnt.cellEnd;
        }

        if (settings->getToolTipsShowImageType())
        {
            tip += cnt.cellBeg + i18n("Type:") + cnt.cellMid + info.mime + cnt.cellEnd;
        }

        if (settings->getToolTipsShowImageDim())
        {
            if ((info.width == 0) || (info.height == 0) || (info.width == -1) || (info.height == -1))
            {
                str = i18nc("unknown / invalid image dimension",
                            "Unknown");
            }
            else
            {
                QString mpixels = QLocale().toString(info.width*info.height/1000000.0, 'f', 1);
                str = i18nc("width x height (megapixels Mpx)", "%1x%2 (%3Mpx)",
                            info.width, info.height, mpixels);
            }

            tip += cnt.cellBeg + i18n("Dimensions:") + cnt.cellMid + str + cnt.cellEnd;
        }
    }

    // -- Photograph Info -----------------------------------------------------------------------

    // NOTE: these info require \"Use File Metadata\" option from Camera Setup Behavior page.

    if (settings->getToolTipsShowPhotoMake()  ||
        settings->getToolTipsShowPhotoLens()  ||
        settings->getToolTipsShowPhotoFocal() ||
        settings->getToolTipsShowPhotoExpo()  ||
        settings->getToolTipsShowPhotoFlash() ||
        settings->getToolTipsShowPhotoWB())
    {
        if (!photoInfo.isNull())
        {
            QString metaStr;
            tip += cnt.headBeg + i18n("Photograph Properties") + cnt.headEnd;

            if (settings->getToolTipsShowPhotoMake())
            {
                ItemPropertiesTab::shortenedMakeInfo(photoInfo.make);
                ItemPropertiesTab::shortenedModelInfo(photoInfo.model);

                str = QString::fromUtf8("%1 / %2").arg(photoInfo.make.isEmpty() ? cnt.unavailable : photoInfo.make)
                      .arg(photoInfo.model.isEmpty() ? cnt.unavailable : photoInfo.model);

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Make/Model:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoLens())
            {
                str          = photoInfo.lens.isEmpty() ? cnt.unavailable : photoInfo.lens;
                QString lens = i18nc("camera lens", "Lens:");

                if (str.length() > cnt.maxStringLength)
                {
                    int space = str.lastIndexOf(QLatin1Char(' '), cnt.maxStringLength);

                    if (space == -1)
                        space = cnt.maxStringLength;

                    metaStr += cnt.cellBeg + lens + cnt.cellMid + str.left(space).toHtmlEscaped() + cnt.cellEnd;

                    str  = str.mid(space+1);
                    lens = QString();
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + lens + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoFocal())
            {
                str = photoInfo.aperture.isEmpty() ? cnt.unavailable : photoInfo.aperture;

                if (photoInfo.focalLength35mm.isEmpty())
                {
                    str += QString::fromUtf8(" / %1").arg(photoInfo.focalLength.isEmpty() ? cnt.unavailable : photoInfo.focalLength);
                }
                else
                {
                    str += QString::fromUtf8(" / %1").arg(i18n("%1 (%2)",photoInfo.focalLength, photoInfo.focalLength35mm));
                }

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Aperture/Focal:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoExpo())
            {
                str = QString::fromUtf8("%1 / %2").arg(photoInfo.exposureTime.isEmpty() ? cnt.unavailable : photoInfo.exposureTime)
                      .arg(photoInfo.sensitivity.isEmpty() ? cnt.unavailable : i18n("%1 ISO",photoInfo.sensitivity));

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("Exposure/Sensitivity:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoFlash())
            {
                str = photoInfo.flash.isEmpty() ? cnt.unavailable : photoInfo.flash;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18nc("camera flash settings",
                                               "Flash:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            if (settings->getToolTipsShowPhotoWB())
            {
                str = photoInfo.whiteBalance.isEmpty() ? cnt.unavailable : photoInfo.whiteBalance;

                if (str.length() > cnt.maxStringLength)
                {
                    str = str.left(cnt.maxStringLength-3) + QLatin1String("...");
                }

                metaStr += cnt.cellBeg + i18n("White Balance:") + cnt.cellMid + str.toHtmlEscaped() + cnt.cellEnd;
            }

            tip += metaStr;
        }
    }

    tip += cnt.tipFooter;

    return tip;
}

} // namespace Digikam
