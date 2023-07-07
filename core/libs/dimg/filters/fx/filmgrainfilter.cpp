/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-25
 * Description : filter to add Film Grain to image.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Julien Narboux <julien at narboux dot fr>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filmgrainfilter.h"

// C++ includes

#include <cstdlib>
#include <cmath>

// Qt includes

#include <QtConcurrent>    // krazy:exclude=includes
#include <QMutex>

// Local includes

#include "dimg.h"
#include "randomnumbergenerator.h"
#include "digikam_globals_p.h"      // For KF6::Ki18n deprecated

namespace Digikam
{

class Q_DECL_HIDDEN FilmGrainFilter::Private
{

public:

    enum YUVChannel
    {
        Luma = 0,
        ChromaBlue,
        ChromaRed
    };

public:

    explicit Private()
      : div(0.0),
        leadLumaNoise(1.0),
        leadChromaBlueNoise(1.0),
        leadChromaRedNoise(1.0),
        globalProgress(0)
    {
    }

    double                div;
    double                leadLumaNoise;
    double                leadChromaBlueNoise;
    double                leadChromaRedNoise;

    FilmGrainContainer    settings;

    RandomNumberGenerator generator;

    int                   globalProgress;

    QMutex                lock;
    QMutex                lock2; // RandomNumberGenerator is not re-entrant (dixit Boost lib)
};

FilmGrainFilter::FilmGrainFilter(QObject* const parent)
    : DImgThreadedFilter(parent),
      d(new Private)
{
    initFilter();
}

FilmGrainFilter::FilmGrainFilter(DImg* const orgImage, QObject* const parent, const FilmGrainContainer& settings)
    : DImgThreadedFilter(orgImage, parent, QLatin1String("FilmGrain")),
      d(new Private)
{
    d->settings = settings;
    initFilter();
}

FilmGrainFilter::FilmGrainFilter(DImgThreadedFilter* const parentFilter,
                                 const DImg& orgImage, const DImg& destImage,
                                 int progressBegin, int progressEnd,
                                 const FilmGrainContainer& settings)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + QLatin1String(": FilmGrain")),
    d(new Private)
{
    d->settings = settings;
    this->filterImage();
}

FilmGrainFilter::~FilmGrainFilter()
{
    cancelFilter();
    delete d;
}

QString FilmGrainFilter::DisplayableName()
{
    return QString::fromUtf8(I18N_NOOP("Film Grain Effect"));
}

void FilmGrainFilter::filmgrainMultithreaded(uint start, uint stop)
{
    // To emulate grain size we use a matrix [grainSize x grainSize].
    // We will parse whole image using grainSize step. Color from a reference point located
    // on the top left corner of matrix will be used to apply noise on whole matrix.
    // In first, for each matrix processed over the image, we compute the lead noise value
    // using Uniform noise generator.
    // In second time, all others points from the matrix are process to add supplemental noise
    // generated with Gaussian or Poisson noise generator.

    DColor refCol, matCol;
    uint    progress=0, oldProgress=0, posX, posY;

    // Reference point noise adjustments.
    double refLumaNoise       = 0.0, refLumaRange       = 0.0;
    double refChromaBlueNoise = 0.0, refChromaBlueRange = 0.0;
    double refChromaRedNoise  = 0.0, refChromaRedRange  = 0.0;

    // Current matrix point noise adjustments.
    double matLumaNoise       = 0.0, matLumaRange       = 0.0;
    double matChromaBlueNoise = 0.0, matChromaBlueRange = 0.0;
    double matChromaRedNoise  = 0.0, matChromaRedRange  = 0.0;

    uint   width  = m_orgImage.width();
    uint   height = m_orgImage.height();

    for (uint x = start ; runningFlag() && (x < stop) ; x += d->settings.grainSize)
    {
        for (uint y = 0 ; runningFlag() && y < height ; y += d->settings.grainSize)
        {
            refCol = m_orgImage.getPixelColor(x, y);
            computeNoiseSettings(refCol,
                                 refLumaRange,       refLumaNoise,
                                 refChromaBlueRange, refChromaBlueNoise,
                                 refChromaRedRange,  refChromaRedNoise);

            // Grain size matrix processing.

            for (int zx = 0 ; runningFlag() && zx < d->settings.grainSize ; ++zx)
            {
                for (int zy = 0 ; runningFlag() && zy < d->settings.grainSize ; ++zy)
                {
                    posX = x + zx;
                    posY = y + zy;

                    if ((posX < width) && (posY < height))
                    {
                        matCol = m_orgImage.getPixelColor(posX, posY);

                        computeNoiseSettings(matCol,
                                             matLumaRange,       matLumaNoise,
                                             matChromaBlueRange, matChromaBlueNoise,
                                             matChromaRedRange,  matChromaRedNoise);

                        if (d->settings.addLuminanceNoise)
                        {
                            if (((refLumaRange - matLumaRange) / refLumaRange) > 0.1)
                            {
                                adjustYCbCr(matCol, matLumaRange, matLumaNoise, Private::Luma);
                            }
                            else
                            {
                                adjustYCbCr(matCol, refLumaRange, refLumaNoise, Private::Luma);
                            }
                        }

                        if (d->settings.addChrominanceBlueNoise)
                        {
                            if (((refChromaBlueRange - matChromaBlueRange) / refChromaBlueRange) > 0.1)
                            {
                                adjustYCbCr(matCol, matChromaBlueRange, matChromaBlueNoise, Private::ChromaBlue);
                            }
                            else
                            {
                                adjustYCbCr(matCol, refChromaBlueRange, refChromaBlueNoise, Private::ChromaBlue);
                            }
                        }

                        if (d->settings.addChrominanceRedNoise)
                        {
                            if (((refChromaRedRange - matChromaRedRange) / refChromaRedRange) > 0.1)
                            {
                                adjustYCbCr(matCol, matChromaRedRange, matChromaRedNoise, Private::ChromaBlue);
                            }
                            else
                            {
                                adjustYCbCr(matCol, refChromaRedRange, refChromaRedNoise, Private::ChromaRed);
                            }
                        }

                        m_destImage.setPixelColor(posX, posY, matCol);
                    }
                }
            }
        }

        progress = (int)( ( (double)x * (100.0 / QThreadPool::globalInstance()->maxThreadCount()) ) / (stop-start));

        if (((progress % 5) == 0) && (progress > oldProgress))
        {
            d->lock.lock();
            oldProgress        = progress;
            d->globalProgress += 5;
            postProgress(d->globalProgress);
            d->lock.unlock();
        }
    }
}

/**
 * This method have been implemented following this report in bugzilla :
 * https://bugs.kde.org/show_bug.cgi?id=148540
 * We use YCbCr color space to perform noise addition. Please follow this url for
 * details about this color space :
 * en.allexperts.com/e/y/yc/ycbcr.htm
 */
void FilmGrainFilter::filterImage()
{
    if ((d->settings.lumaIntensity <= 0)       ||
        (d->settings.chromaBlueIntensity <= 0) ||
        (d->settings.chromaRedIntensity <= 0)  ||
        !d->settings.isDirty())
    {
        m_destImage = m_orgImage;
        return;
    }

    d->div                 = m_orgImage.sixteenBit() ? 65535.0 : 255.0;
    d->leadLumaNoise       = d->settings.lumaIntensity       * (m_orgImage.sixteenBit() ? 256.0 : 1.0);
    d->leadChromaBlueNoise = d->settings.chromaBlueIntensity * (m_orgImage.sixteenBit() ? 256.0 : 1.0);
    d->leadChromaRedNoise  = d->settings.chromaRedIntensity  * (m_orgImage.sixteenBit() ? 256.0 : 1.0);

    d->generator.seed(1); // noise will always be the same

    QList<int> vals = multithreadedSteps(m_orgImage.width());
    QList <QFuture<void> > tasks;

    for (int j = 0 ; runningFlag() && (j < vals.count()-1) ; ++j)
    {
        tasks.append(QtConcurrent::run(
#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
                                       &FilmGrainFilter::filmgrainMultithreaded, this,
#else
                                       this, &FilmGrainFilter::filmgrainMultithreaded,
#endif
                                       vals[j],
                                       vals[j+1]
                                      )
        );
    }

    Q_FOREACH (QFuture<void> t, tasks)
    {
        t.waitForFinished();
    }
}

/**
 * This method compute lead noise of reference matrix point used to simulate graininess size
 */
void FilmGrainFilter::computeNoiseSettings(const DColor& col,
                                           double& luRange, double& luNoise,
                                           double& cbRange, double& cbNoise,
                                           double& crRange, double& crNoise)
{
    if (d->settings.addLuminanceNoise)
    {
        luRange = interpolate(d->settings.lumaShadows, d->settings.lumaMidtones,
                              d->settings.lumaHighlights, col) * d->leadLumaNoise + 1.0;
        luNoise = randomizeUniform(luRange);
    }

    if (d->settings.addChrominanceBlueNoise)
    {
        cbRange = interpolate(d->settings.chromaBlueShadows, d->settings.chromaBlueMidtones,
                              d->settings.chromaBlueHighlights, col) * d->leadChromaBlueNoise + 1.0;
        cbNoise = randomizeUniform(cbRange);
    }

    if (d->settings.addChrominanceRedNoise)
    {
        crRange = interpolate(d->settings.chromaRedShadows, d->settings.chromaRedMidtones,
                              d->settings.chromaRedHighlights, col) * d->leadChromaRedNoise + 1.0;
        crNoise = randomizeUniform(crRange);
    }
}

/**
 * This method apply grain adjustment on a pixel color channel from YCrCb color space.
 * NRand is the lead uniform noise set from matrix used to scan whole image step by step.
 * Additionally noise is applied on pixel using Poisson or Gaussian distribution.
 */
void FilmGrainFilter::adjustYCbCr(DColor& col, double range, double nRand, int channel)
{
    double y, cb, cr, n2;
    col.getYCbCr(&y, &cb, &cr);

    if (d->settings.photoDistribution)
    {
        n2 = randomizePoisson((d->settings.grainSize / 2.0) * (range / 1.414));
    }
    else
    {
        n2 = randomizeGauss((d->settings.grainSize / 2.0) * (range / 1.414));
    }

    switch (channel)
    {
        case Private::Luma:
            y  = CLAMP(y  + (nRand + n2) / d->div, 0.0, 1.0);
            break;

        case Private::ChromaBlue:
            cb = CLAMP(cb + (nRand + n2) / d->div, 0.0, 1.0);
            break;

        default:       // ChromaRed
            cr = CLAMP(cr + (nRand + n2) / d->div, 0.0, 1.0);
            break;
    }

    col.setYCbCr(y, cb, cr, col.sixteenBit());
}

/**
 * This method compute uniform noise value used to randomize matrix reference point.
 * This value is lead noise apply to image.
 */
double FilmGrainFilter::randomizeUniform(double range)
{
    d->lock2.lock();
    double val = d->generator.number(- range / 2, range / 2);
    d->lock2.unlock();

    return val;
}

/**
 * This method compute Gaussian noise value used to randomize all matrix points.
 * This value is added to lead noise value.
 */
double FilmGrainFilter::randomizeGauss(double sigma)
{
    d->lock2.lock();
    double u = - d->generator.number(-1.0, 0.0); // exclude 0
    double v = d->generator.number(0.0, 1.0);
    d->lock2.unlock();
    return (sigma * sqrt(-2 * log(u)) * cos(2 * M_PI * v));
}

/**
 * This method compute Poisson noise value used to randomize all matrix points.
 * This value is added to lead noise value.
 * Poisson noise is more realist to simulate photon noise apply on analog film.
 * NOTE: see approximation of Poisson noise using Gauss algorithm from noise.c code take from :
 *       registry.gimp.org/node/13016
 *       This method is very fast compared to real Poisson noise generator.
 */
double FilmGrainFilter::randomizePoisson(double lambda)
{
    return (randomizeGauss(sqrt(lambda * d->settings.grainSize * d->settings.grainSize)));
}

/**
 * This method interpolate gain adjustments to apply grain on shadows, midtones and highlights colors.
 * The output value is a coefficient computed between 0.0 and 1.0.
 */
double FilmGrainFilter::interpolate(int shadows, int midtones, int highlights, const DColor& col)
{
    double s = (shadows    + 100.0) / 200.0;
    double m = (midtones   + 100.0) / 200.0;
    double h = (highlights + 100.0) / 200.0;

    double y, cb, cr;
    col.getYCbCr(&y, &cb, &cr);

    if (y >= 0.0 && y <= 0.5)
    {
        return (s + 2 * (m - s) * y);
    }
    else if (y >= 0.5 && y <= 1.0)
    {
        return (2 * (h - m) * y + 2 * m - h);
    }
    else
    {
        return 1.0;
    }
}

FilterAction FilmGrainFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    action.addParameter(QLatin1String("grainSize"),               d->settings.grainSize);
    action.addParameter(QLatin1String("photoDistribution"),       d->settings.photoDistribution);

    action.addParameter(QLatin1String("addLuminanceNoise"),       d->settings.addLuminanceNoise);
    action.addParameter(QLatin1String("lumaIntensity"),           d->settings.lumaIntensity);
    action.addParameter(QLatin1String("lumaShadows"),             d->settings.lumaShadows);
    action.addParameter(QLatin1String("lumaMidtones"),            d->settings.lumaMidtones);
    action.addParameter(QLatin1String("lumaHighlights"),          d->settings.lumaHighlights);

    action.addParameter(QLatin1String("addChrominanceBlueNoise"), d->settings.addChrominanceBlueNoise);
    action.addParameter(QLatin1String("chromaBlueIntensity"),     d->settings.chromaBlueIntensity);
    action.addParameter(QLatin1String("chromaBlueShadows"),       d->settings.chromaBlueShadows);
    action.addParameter(QLatin1String("chromaBlueMidtones"),      d->settings.chromaBlueMidtones);
    action.addParameter(QLatin1String("chromaBlueHighlights"),    d->settings.chromaBlueHighlights);

    action.addParameter(QLatin1String("addChrominanceRedNoise"),  d->settings.addChrominanceRedNoise);
    action.addParameter(QLatin1String("chromaRedIntensity"),      d->settings.chromaRedIntensity);
    action.addParameter(QLatin1String("chromaRedShadows"),        d->settings.chromaRedShadows);
    action.addParameter(QLatin1String("chromaRedMidtones"),       d->settings.chromaRedMidtones);
    action.addParameter(QLatin1String("chromaRedHighlights"),     d->settings.chromaRedHighlights);

    return action;
}

void FilmGrainFilter::readParameters(const Digikam::FilterAction& action)
{
    d->settings.grainSize               = action.parameter(QLatin1String("grainSize")).toInt();
    d->settings.photoDistribution       = action.parameter(QLatin1String("photoDistribution")).toBool();

    d->settings.addLuminanceNoise       = action.parameter(QLatin1String("addLuminanceNoise")).toBool();
    d->settings.lumaIntensity           = action.parameter(QLatin1String("lumaIntensity")).toInt();
    d->settings.lumaShadows             = action.parameter(QLatin1String("lumaShadows")).toInt();
    d->settings.lumaMidtones            = action.parameter(QLatin1String("lumaMidtones")).toInt();
    d->settings.lumaHighlights          = action.parameter(QLatin1String("lumaHighlights")).toInt();

    d->settings.addChrominanceBlueNoise = action.parameter(QLatin1String("addChrominanceBlueNoise")).toBool();
    d->settings.chromaBlueIntensity     = action.parameter(QLatin1String("chromaBlueIntensity")).toInt();
    d->settings.chromaBlueShadows       = action.parameter(QLatin1String("chromaBlueShadows")).toInt();
    d->settings.chromaBlueMidtones      = action.parameter(QLatin1String("chromaBlueMidtones")).toInt();
    d->settings.chromaBlueHighlights    = action.parameter(QLatin1String("chromaBlueHighlights")).toInt();

    d->settings.addChrominanceRedNoise  = action.parameter(QLatin1String("addChrominanceRedNoise")).toBool();
    d->settings.chromaRedIntensity      = action.parameter(QLatin1String("chromaRedIntensity")).toInt();
    d->settings.chromaRedShadows        = action.parameter(QLatin1String("chromaRedShadows")).toInt();
    d->settings.chromaRedMidtones       = action.parameter(QLatin1String("chromaRedMidtones")).toInt();
    d->settings.chromaRedHighlights     = action.parameter(QLatin1String("chromaRedHighlights")).toInt();
}

} // namespace Digikam
