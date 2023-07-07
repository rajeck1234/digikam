/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : A Sharpen threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SHARPEN_FILTER_H
#define DIGIKAM_SHARPEN_FILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT SharpenFilter : public DImgThreadedFilter
{
    Q_OBJECT

private:

    struct Q_DECL_HIDDEN Args
    {
        explicit Args()
          : start(0),
            stop(0),
            y(0),
            kernelWidth(0),
            normal_kernel(nullptr),
            halfKernelWidth(0)
        {
        }

        uint    start;
        uint    stop;
        uint    y;
        long    kernelWidth;
        double* normal_kernel;
        long    halfKernelWidth;
    };

public:

    explicit SharpenFilter(QObject* const parent = nullptr);
    explicit SharpenFilter(DImg* const orgImage, QObject* const parent = nullptr, double radius=0.0, double sigma=1.0);

    /**
     * Constructor for slave mode: execute immediately in current thread with specified master filter
     */
    SharpenFilter(DImgThreadedFilter* const parentFilter, const DImg& orgImage, const DImg& destImage,
                  int progressBegin=0, int progressEnd=100, double radius=0.0, double sigma=1.0);

    ~SharpenFilter() override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:SharpenFilter");
    }

    static QString          DisplayableName();

    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int              CurrentVersion()
    {
        return 1;
    }

    QString         filterIdentifier()                          const override
    {
        return FilterIdentifier();
    }

    FilterAction    filterAction()                                    override;
    void                    readParameters(const FilterAction& action)        override;


private:

    void filterImage()                                                        override;

    void sharpenImage(double radius, double sigma);

    bool convolveImage(const unsigned int order, const double* const kernel);

    void convolveImageMultithreaded(const Args& prm);

    int  getOptimalKernelWidth(double radius, double sigma);

private:

    double m_radius;
    double m_sigma;
};

} // namespace Digikam

#endif // DIGIKAM_SHARPEN_FILTER_H
