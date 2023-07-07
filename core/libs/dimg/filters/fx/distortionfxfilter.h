/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-18
 * Description : Distortion FX threaded image filter.
 *
 * SPDX-FileCopyrightText: 2005-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010      by Martin Klapetek <martin dot klapetek at gmail dot com>
 *
 * Original Distortion algorithms copyrighted 2004-2005 by
 * Pieter Z. Voloshyn <pieter dot voloshyn at gmail dot com>.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DISTORTION_FX_FILTER_H
#define DIGIKAM_DISTORTION_FX_FILTER_H

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "digikam_globals.h"

namespace Digikam
{

class DIGIKAM_EXPORT DistortionFXFilter : public DImgThreadedFilter
{
    Q_OBJECT

public:

    enum DistortionFXTypes
    {
        FishEye=0,
        Twirl,
        CilindricalHor,
        CilindricalVert,
        CilindricalHV,
        Caricature,
        MultipleCorners,
        WavesHorizontal,
        WavesVertical,
        BlockWaves1,
        BlockWaves2,
        CircularWaves1,
        CircularWaves2,
        PolarCoordinates,
        UnpolarCoordinates,
        Tile
    };

private:

    struct Q_DECL_HIDDEN Args
    {
        explicit Args()
          : start(0),
            stop(0),
            h(0),
            w(0),
            orgImage(nullptr),
            destImage(nullptr),
            Coeff(0.0),
            AntiAlias(false),
            dist(0),
            Horizontal(false),
            Vertical(false),
            Factor(0),
            Amplitude(0),
            Frequency(0),
            Mode(false),
            X(0),
            Y(0),
            Phase(0.0),
            WavesType(false),
            FillSides(false),
            Type(false),
            WSize(0),
            HSize(0),
            Random(0)
        {
        }

        int    start;
        int    stop;
        int    h;
        int    w;
        DImg*  orgImage;
        DImg*  destImage;
        double Coeff;
        bool   AntiAlias;
        int    dist;
        bool   Horizontal;
        bool   Vertical;
        int    Factor;
        int    Amplitude;
        int    Frequency;
        bool   Mode;
        int    X;
        int    Y;
        double Phase;
        bool   WavesType;
        bool   FillSides;
        bool   Type;
        int    WSize;
        int    HSize;
        int    Random;
    };

public:

    explicit DistortionFXFilter(QObject* const parent = nullptr);
    explicit DistortionFXFilter(DImg* const orgImage, QObject* const parent = nullptr, int effectType=0,
                                int level=0, int iteration=0, bool antialiasing=true);

    ~DistortionFXFilter() override;

    static QString          FilterIdentifier()
    {
        return QLatin1String("digikam:DistortionFXFilter");
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

    // Backported from ImageProcessing version 2
    void fisheye(DImg* orgImage, DImg* destImage, double Coeff, bool AntiAlias=true);
    void fisheyeMultithreaded(const Args& prm);

    void twirl(DImg* orgImage, DImg* destImage, int dist, bool AntiAlias=true);
    void twirlMultithreaded(const Args& prm);

    void cilindrical(DImg* orgImage, DImg* destImage, double Coeff,
                     bool Horizontal, bool Vertical, bool AntiAlias=true);
    void cilindricalMultithreaded(const Args& prm);

    void multipleCorners(DImg* orgImage, DImg* destImage, int Factor, bool AntiAlias=true);
    void multipleCornersMultithreaded(const Args& prm);

    void polarCoordinates(DImg* orgImage, DImg* destImage, bool Type, bool AntiAlias=true);
    void polarCoordinatesMultithreaded(const Args& prm);

    void circularWaves(DImg* orgImage, DImg* destImage, int X, int Y, double Amplitude,
                       double Frequency, double Phase, bool WavesType, bool AntiAlias=true);
    void circularWavesMultithreaded(const Args& prm);

    // Backported from ImageProcessing version 1
    void waves(DImg* orgImage, DImg* destImage, int Amplitude, int Frequency,
               bool FillSides, bool Direction);
    void wavesHorizontalMultithreaded(const Args& prm);
    void wavesVerticalMultithreaded(const Args& prm);

    void blockWaves(DImg* orgImage, DImg* destImage, int Amplitude, int Frequency, bool Mode);
    void blockWavesMultithreaded(const Args& prm);

    void tile(DImg* orgImage, DImg* destImage, int WSize, int HSize, int Random);
    void tileMultithreaded(const Args& prm);

    void setPixelFromOther(int Width, int Height, bool sixteenBit, int bytesDepth,
                           uchar* data, uchar* pResBits,
                           int w, int h, double nw, double nh, bool AntiAlias);

    // Return the limit defined the max and min values.
    inline int Lim_Max(int Now, int Up, int Max)
    {
        --Max;

        while (Now > Max - Up)
        {
            --Up;
        }

        return (Up);
    };

    inline bool isInside (int Width, int Height, int X, int Y)
    {
        bool bIsWOk = ((X < 0) ? false : (X >= Width ) ? false : true);
        bool bIsHOk = ((Y < 0) ? false : (Y >= Height) ? false : true);

        return (bIsWOk && bIsHOk);
    };

    inline int getOffset(int Width, int X, int Y, int bytesDepth)
    {
        return (Y * Width * bytesDepth) + (X * bytesDepth);
    };

    inline int getOffsetAdjusted(int Width, int Height, int X, int Y, int bytesDepth)
    {
        X = (X < 0) ?  0  :  ((X >= Width ) ? (Width  - 1) : X);
        Y = (Y < 0) ?  0  :  ((Y >= Height) ? (Height - 1) : Y);

        return getOffset(Width, X, Y, bytesDepth);
    };

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DISTORTION_FX_FILTER_H
