/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : Wavelets Noise Reduction threaded image filter.
 *               This filter work in YCrCb color space.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_NR_FILTER_H
#define DIGIKAM_NR_FILTER_H

// Qt includes

#include <QMetaType>
#include <QDebug>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"

namespace Digikam
{

class DIGIKAM_EXPORT NRContainer
{

public:

    explicit NRContainer();
    ~NRContainer();

public:

    /**
     * Separated values per channel
     */
    double thresholds[3];    ///< Y, Cb, Cr thresholds.
    double softness[3];      ///< Y, Cb, Cr softness.
};

//! qDebug() stream operator. Writes property @a inf to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const NRContainer& inf);

// --------------------------------------------------------------------------

class DIGIKAM_EXPORT NRFilter : public DImgThreadedFilter
{
    Q_OBJECT

private:

    struct Q_DECL_HIDDEN Args
    {
        explicit Args()
          : start       (0),
            stop        (0),
            thold       (nullptr),
            lpass       (nullptr),
            hpass       (nullptr),
            stdev       (nullptr),
            samples     (nullptr),
            fimg        (nullptr),
            threshold   (0.0),
            softness    (0.0)
        {
        }

        uint    start;
        uint    stop;
        float*  thold;
        uint*   lpass;
        uint*   hpass;
        double* stdev;
        uint*   samples;
        float** fimg;
        float   threshold;
        double  softness;
    };

public:

    explicit NRFilter(QObject* const parent = nullptr);
    NRFilter(DImg* const orgImage, QObject* const parent, const NRContainer& settings);
    ~NRFilter()                                           override;

    void readParameters(const FilterAction& action)       override;

    FilterAction filterAction()                           override;
    QString filterIdentifier()                      const override;

    static QString FilterIdentifier();
    static QString DisplayableName();
    static QList<int> SupportedVersions();
    static int CurrentVersion();

    static void srgb2ycbcr(float** const fimg, int size);

private:

    void filterImage()                                    override;

    void waveletDenoise(float* fimg[3], unsigned int width, unsigned int height,
                        float threshold, double softness);
    inline void hatTransform(float* const temp, float* const base, int st, int size, int sc);

    void ycbcr2srgb(float** const fimg, int size);

    void calculteStdevMultithreaded(const Args& prm);
    void thresholdingMultithreaded(const Args& prm);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_NR_FILTER_H */
