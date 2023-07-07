/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : equalize image filter.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "equalizefilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "imagehistogram.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

EqualizeFilter::EqualizeFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

EqualizeFilter::EqualizeFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("EqualizeFilter")),
      m_refImage(*refImage)
{
    initFilter();
}

EqualizeFilter::~EqualizeFilter()
{
}

QString EqualizeFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Auto Equalize"));
}

void EqualizeFilter::filterImage()
{
    if (m_refImage.isNull())
    {
        m_refImage = m_orgImage;
    }

    equalizeImage();
    m_destImage = m_orgImage;
}

/**
 * Performs an histogram equalization of the image.
 * this method adjusts the brightness of colors across the
 * active image so that the histogram for the value channel
 * is as nearly as possible flat, that is, so that each possible
 * brightness value appears at about the same number of pixels
 * as each other value. Sometimes Equalize works wonderfully at
 * enhancing the contrasts in an image. Other times it gives
 * garbage. It is a very powerful operation, which can either work
 * miracles on an image or destroy it.
 */
void EqualizeFilter::equalizeImage()
{
    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Ref. image and Org. has different bits depth";
        return;
    }

    struct double_packet  high, low, intensity;
    int                   i;
    int                   progress;

    // Create an histogram of the reference image.

    QScopedPointer<ImageHistogram> histogram(new ImageHistogram(m_refImage));
    histogram->calculate();

    // Memory allocation.

    QScopedArrayPointer<double_packet> map(new double_packet[histogram->getHistogramSegments()]);
    QScopedArrayPointer<int_packet> equalize_map(new int_packet[histogram->getHistogramSegments()]);

    if (map.isNull() || equalize_map.isNull())
    {
        qCWarning(DIGIKAM_DIMG_LOG) << ("Unable to allocate memory!");
        return;
    }

    // Integrate the histogram to get the equalization map.

    for (i = 0 ; runningFlag() && (i < histogram->getHistogramSegments()) ; ++i)
    {
        intensity.red   += histogram->getValue(RedChannel,   i);
        intensity.green += histogram->getValue(GreenChannel, i);
        intensity.blue  += histogram->getValue(BlueChannel,  i);
        intensity.alpha += histogram->getValue(AlphaChannel, i);
        map[i]           = intensity;
    }

    // Stretch the histogram.

    low  = map[0];
    high = map[histogram->getHistogramSegments() - 1];

    // TODO magic number 256

    for (i = 0 ; runningFlag() && (i < histogram->getHistogramSegments()) ; ++i)
    {
        if (high.red != low.red)
            equalize_map[i].red = (uint)(((256 * histogram->getHistogramSegments() - 1) *
                                          (map[i].red - low.red)) / (high.red - low.red));

        if (high.green != low.green)
            equalize_map[i].green = (uint)(((256 * histogram->getHistogramSegments() - 1) *
                                            (map[i].green - low.green)) / (high.green - low.green));

        if (high.blue != low.blue)
            equalize_map[i].blue = (uint)(((256 * histogram->getHistogramSegments() - 1) *
                                           (map[i].blue - low.blue)) / (high.blue - low.blue));

        if (high.alpha != low.alpha)
            equalize_map[i].alpha = (uint)(((256 * histogram->getHistogramSegments() - 1) *
                                            (map[i].alpha - low.alpha)) / (high.alpha - low.alpha));
    }

    uchar* data     = m_orgImage.bits();
    int w           = m_orgImage.width();
    int h           = m_orgImage.height();
    bool sixteenBit = m_orgImage.sixteenBit();
    int size        = w * h;

    // Apply results to image.
    // TODO magic number 257

    if (!sixteenBit)        // 8 bits image.
    {
        uchar  red, green, blue, alpha;
        uchar* ptr = data;

        for (i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (low.red != high.red)
            {
                red = (equalize_map[red].red) / 257;
            }

            if (low.green != high.green)
            {
                green = (equalize_map[green].green) / 257;
            }

            if (low.blue != high.blue)
            {
                blue = (equalize_map[blue].blue) / 257;
            }

            if (low.alpha != high.alpha)
            {
                alpha = (equalize_map[alpha].alpha) / 257;
            }

            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr   += 4;

            progress = (int)(((double)i * 100.0) / size);

            if ((progress % 5) == 0)
            {
                postProgress(progress);
            }
        }
    }
    else               // 16 bits image.
    {
        unsigned short  red, green, blue, alpha;
        unsigned short* ptr = reinterpret_cast<unsigned short*>(data);

        for (i = 0 ; runningFlag() && (i < size) ; ++i)
        {
            blue  = ptr[0];
            green = ptr[1];
            red   = ptr[2];
            alpha = ptr[3];

            if (low.red != high.red)
            {
                red = (equalize_map[red].red) / 257;
            }

            if (low.green != high.green)
            {
                green = (equalize_map[green].green) / 257;
            }

            if (low.blue != high.blue)
            {
                blue = (equalize_map[blue].blue) / 257;
            }

            if (low.alpha != high.alpha)
            {
                alpha = (equalize_map[alpha].alpha) / 257;
            }

            ptr[0] = blue;
            ptr[1] = green;
            ptr[2] = red;
            ptr[3] = alpha;
            ptr   += 4;

            progress = (int)(((double)i * 100.0) / size);

            if ((progress % 5) == 0)
            {
                postProgress(progress);
            }
        }
    }
}

FilterAction EqualizeFilter::filterAction()
{
    return DefaultFilterAction<EqualizeFilter>();
}

void EqualizeFilter::readParameters(const FilterAction& /*action*/)
{
    return;
}

} // namespace Digikam
