/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : Image Quality Parser - noise basic factor detection
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_NOISE_DETECTOR_H
#define DIGIKAM_NOISE_DETECTOR_H

// Local includes

#include "abstract_detector.h"

namespace Digikam
{

class NoiseDetector : public AbstractDetector
{
    Q_OBJECT

public:

    typedef QList<cv::Mat> Mat3D;

public:

    explicit NoiseDetector();
    ~NoiseDetector();

    float detect(const cv::Mat& image)                                          const override;

public:

    static const Mat3D filtersHaar;

private:

    Mat3D   decompose_by_filter(const cv::Mat& image, const Mat3D& filters)     const;
    void    calculate_variance_kurtosis(const Mat3D& channels,
                                        cv::Mat& variance,
                                        cv::Mat& kurtosis)                      const;
    float   noise_variance(const cv::Mat& variance, const cv::Mat& kurtosis)    const;
    float   normalize(const float number)                                       const;

    cv::Mat raw_moment(const NoiseDetector::Mat3D& mat, int order)              const;
    cv::Mat pow_mat(const cv::Mat& mat, float ordre)                            const;
    float   mean_mat(const cv::Mat& mat)                                        const;

    // Disable
    explicit NoiseDetector(QObject*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_NOISE_DETECTOR_H
