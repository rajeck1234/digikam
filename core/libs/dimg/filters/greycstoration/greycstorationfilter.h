/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-12-03
 * Description : Greycstoration interface.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_GREY_CSTORATION_FILTER_H
#define DIGIKAM_GREY_CSTORATION_FILTER_H

// Qt includes

#include <QImage>

// Local includes

#include "dimg.h"
#include "dimgthreadedfilter.h"
#include "digikam_export.h"

class QObject;

namespace Digikam
{

class DIGIKAM_EXPORT GreycstorationContainer
{

public:

    enum INTERPOLATION
    {
        NearestNeighbor = 0,
        Linear,
        RungeKutta
    };

public:

    explicit GreycstorationContainer()
    {
        setRestorationDefaultSettings();
    };

    ~GreycstorationContainer()
    {
    };

    void setRestorationDefaultSettings()
    {
        fastApprox = true;

        tile       = 256;
        btile      = 4;

        nbIter     = 1;
        interp     = NearestNeighbor;

        amplitude  = 60.0;
        sharpness  = 0.7F;
        anisotropy = 0.3F;
        alpha      = 0.6F;
        sigma      = 1.1F;
        gaussPrec  = 2.0;
        dl         = 0.8F;
        da         = 30.0;
    };

    void setInpaintingDefaultSettings()
    {
        fastApprox = true;

        tile       = 256;
        btile      = 4;

        nbIter     = 30;
        interp     = NearestNeighbor;

        amplitude  = 20.0;
        sharpness  = 0.3F;
        anisotropy = 1.0;
        alpha      = 0.8F;
        sigma      = 2.0;
        gaussPrec  = 2.0;
        dl         = 0.8F;
        da         = 30.0;
    };

    void setResizeDefaultSettings()
    {
        fastApprox = true;

        tile       = 256;
        btile      = 4;

        nbIter     = 3;
        interp     = NearestNeighbor;

        amplitude  = 20.0;
        sharpness  = 0.2F;
        anisotropy = 0.9F;
        alpha      = 0.1F;
        sigma      = 1.5;
        gaussPrec  = 2.0;
        dl         = 0.8F;
        da         = 30.0;
    };

public:

    bool  fastApprox;

    int   tile;
    int   btile;

    uint  nbIter;
    uint  interp;

    float amplitude;
    float sharpness;
    float anisotropy;
    float alpha;
    float sigma;
    float gaussPrec;
    float dl;
    float da;
};

// --------------------------------------------------------------------------

class DIGIKAM_EXPORT GreycstorationFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    enum MODE
    {
        Restore = 0,
        InPainting,
        Resize,
        SimpleResize    ///< Mode to resize image without to use Greycstoration algorithm.
    };

public:

    /**
     * Constructor without argument. Before to use it,
     * you need to call in order: setSettings(), setMode(), optionally setInPaintingMask(),
     * setOriginalImage(), and necessary setup() at end.
     */
    explicit GreycstorationFilter(QObject* const parent = nullptr);

    /**
     * Constructor with all arguments. Ready to use.
     */
    GreycstorationFilter(DImg* const orgImage,
                         const GreycstorationContainer& settings,
                         int mode = Restore,
                         int newWidth = 0, int newHeight = 0,
                         const QImage& inPaintingMask = QImage(),
                         QObject* const parent = nullptr);

    ~GreycstorationFilter()                                   override;

    void setMode(int mode, int newWidth = 0, int newHeight = 0);
    void setSettings(const GreycstorationContainer& settings);
    void setInPaintingMask(const QImage& inPaintingMask);

    void setup();

    void cancelFilter() override;

    static QString cimgVersionString();

    static QString FilterIdentifier()
    {
        return QLatin1String("digikam:GreycstorationFilter");
    }

    static QString DisplayableName();

    static QList<int> SupportedVersions()
    {
        return QList<int>() << 1;
    }

    static int CurrentVersion()
    {
        return 1;
    }

    QString filterIdentifier()                          const override
    {
        return FilterIdentifier();
    }

    FilterAction filterAction()                               override;
    void readParameters(const FilterAction& action)           override;

private:

    void computeChildrenThreads();
    void restoration();
    void inpainting();
    void resize();
    void simpleResize();
    void iterationLoop(uint iter);

    void initFilter()                                         override;
    void filterImage()                                        override;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_GREY_CSTORATION_FILTER_H
