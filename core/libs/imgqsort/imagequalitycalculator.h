/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 25/08/2013
 * Description : Image Quality Calculor
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_QUALITY_CALCULATOR_H
#define DIGIKAM_IMAGE_QUALITY_CALCULATOR_H

// Local includes

#include "digikam_globals.h"

// Qt includes

#include <QString>
#include <QList>

namespace Digikam
{

class ImageQualityCalculator
{
public:

    struct ResultDetection
    {
        QString detetionType;
        float   weight;
        float   score;
    };

public:

    explicit ImageQualityCalculator();
    ~ImageQualityCalculator();

    float calculateQuality()                    const;
    void addDetectionResult(const QString& name,
                            const float score,
                            const float weight) const;

private:

    void normalizeWeight()                      const;
    void adjustWeightByQualityLevel()           const;
    int  numberDetectors()                      const;

    // Disable
    ImageQualityCalculator(const ImageQualityCalculator&)            = delete;
    ImageQualityCalculator& operator=(const ImageQualityCalculator&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_QUALITY_CALCULATOR_H
