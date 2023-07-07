/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-24-01
 * Description : auto levels image filter.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "autolevelsfilter.h"

// C++ includes

#include <cstdio>
#include <cmath>

// Local includes

#include "dimg.h"
#include "digikam_debug.h"
#include "imagehistogram.h"
#include "imagelevels.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

AutoLevelsFilter::AutoLevelsFilter(QObject* const parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

AutoLevelsFilter::AutoLevelsFilter(DImg* const orgImage, const DImg* const refImage, QObject* const parent)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("AutoLevelsFilter")),
      m_refImage        (*refImage)
{
    initFilter();
}

AutoLevelsFilter::~AutoLevelsFilter()
{
    cancelFilter();
}

QString AutoLevelsFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Auto Levels"));
}

void AutoLevelsFilter::filterImage()
{
    if (m_refImage.isNull())
    {
        m_refImage = m_orgImage;
    }

    autoLevelsCorrectionImage();
    m_destImage = m_orgImage;
}

/**
 * Performs histogram auto correction of levels.
 * This method maximizes the tonal range in the Red,
 * Green, and Blue channels. It search the image shadow and highlight
 * limit values and adjust the Red, Green, and Blue channels
 * to a full histogram range.
 */
void AutoLevelsFilter::autoLevelsCorrectionImage()
{
    if (m_orgImage.sixteenBit() != m_refImage.sixteenBit())
    {
        qCDebug(DIGIKAM_DIMG_LOG) << "Ref. image and Org. has different bits depth";
        return;
    }

    uchar* data     = m_orgImage.bits();
    int w           = m_orgImage.width();
    int h           = m_orgImage.height();
    bool sixteenBit = m_orgImage.sixteenBit();

    QScopedArrayPointer<uchar> desData;
    QScopedPointer<ImageHistogram> histogram;
    QScopedPointer<ImageLevels> levels;

    postProgress(10);

    int sizeSixteenBit = w * h * 8;
    int sizeEightBit   = w * h * 4;

    // Create the new empty destination image data space.

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        if (sixteenBit)
        {
            desData.reset(new uchar[sizeSixteenBit]);
        }
        else
        {
            desData.reset(new uchar[sizeEightBit]);
        }

        postProgress(20);
    }

    // Create an histogram of the reference image.

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        histogram.reset(new ImageHistogram(m_refImage));
        histogram->calculate();
        postProgress(30);
    }

    // Create an empty instance of levels to use.

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        levels.reset(new ImageLevels(sixteenBit));
        postProgress(40);
    }

    // Initialize an auto levels correction of the histogram.

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        levels->levelsAuto(histogram.data());
        postProgress(50);
    }

    // Calculate the LUT to apply on the image.

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        levels->levelsLutSetup(AlphaChannel);
        postProgress(60);
    }

    // Apply the lut to the image.

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        levels->levelsLutProcess(data, desData.data(), w, h);
        postProgress(70);
    }

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        if (sixteenBit)
        {
            memcpy(data, desData.data(), sizeSixteenBit);
        }
        else
        {
            memcpy(data, desData.data(), sizeEightBit);
        }

        postProgress(80);
    }

    // cppcheck-suppress duplicateCondition
    if (runningFlag())
    {
        postProgress(90);
    }
}

FilterAction AutoLevelsFilter::filterAction()
{
    return DefaultFilterAction<AutoLevelsFilter>();
}

void AutoLevelsFilter::readParameters(const FilterAction& /*action*/)
{
    return;
}

} // namespace Digikam
