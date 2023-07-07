/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-04
 * Description : a tool to create panorama by fusion of several images.
 *               This type is based on pto file format described here:
 *               hugin.sourceforge.net/docs/nona/nona.txt, and
 *               on pto files produced by Hugin's tools.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ptotype.h"

// C includes

#include <math.h>

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"

namespace Digikam
{

bool PTOType::createFile(const QString& filepath)
{
    QFile file(filepath);

    if (!file.open(QFile::WriteOnly | QFile::Truncate))
    {
        qCDebug(DIGIKAM_GENERAL_LOG) << "Cannot open " << filepath << " to write the pto file";
        return false;
    }

    QTextStream out(&file);
    out.setRealNumberPrecision(15);

    // First, the pano line

    if (project.previousComments.size() > 0)
    {
        out << project.previousComments.join(QLatin1Char('\n')) << QT_ENDL;
    }

    out << "p";
    out << " f" << project.projection;

    if (project.size.width() > 0)
    {
        out << " w" << project.size.width();
    }

    if (project.size.height() > 0)
    {
        out << " h" << project.size.height();
    }

    if (project.fieldOfView > 0)
    {
        out << " v" << project.fieldOfView;
    }

    out << " k" << project.photometricReferenceId;
    out << " E" << project.exposure;
    out << " R" << project.hdr;

    switch (project.bitDepth)
    {
        case Project::UINT16:
            out << " T\"UINT16\"";
            break;

        case Project::FLOAT:
            out << " T\"FLOAT\"";
            break;

        default:
            break;
    }

    if (project.crop.height() > 1 && project.crop.width() > 1)
    {
        out << " S";
        out << project.crop.left();
        out <<  "," << project.crop.right();
        out <<  "," << project.crop.top();
        out <<  "," << project.crop.bottom();
    }

    out << " n\"";

    switch (project.fileFormat.fileType)
    {
        case Project::FileFormat::PNG:
        {
            out << "PNG";
            break;
        }

        case Project::FileFormat::JPEG:
        {
            out <<  "JPEG";
            out << " q" << project.fileFormat.quality;
            break;
        }

        case Project::FileFormat::TIFF:
        {
            out << "TIFF c:" << project.fileFormat.compressionMethod;
            break;
        }

        case Project::FileFormat::TIFF_m:
        case Project::FileFormat::TIFF_multilayer:
        {
            if (project.fileFormat.fileType == Project::FileFormat::TIFF_multilayer)
            {
                out << "TIFF_multilayer";
            }
            else
            {
                out << "TIFF_m";
            }

            out << " c:";

            switch (project.fileFormat.compressionMethod)
            {
                case Project::FileFormat::PANO_NONE:
                    out << "PANO_NONE";
                    break;

                case Project::FileFormat::LZW:
                    out << "LZW";
                    break;

                case Project::FileFormat::DEFLATE:
                    out << "DEFLATE";
                    break;
            }

            if (project.fileFormat.savePositions)
            {
                out << " p1";
            }

            if (project.fileFormat.cropped)
            {
                out << " r:CROP";
            }

            break;
        }

        default:
        {
            qCCritical(DIGIKAM_GENERAL_LOG) << "Unknown file format for pto file generation!";
            file.close();

            return false;
        }
    }

    out << "\"";
    out << project.unmatchedParameters.join(QLatin1Char(' ')) << QT_ENDL;

    // Second, the stitcher line

    if (stitcher.previousComments.size() > 0)
    {
        out << stitcher.previousComments.join(QLatin1Char('\n')) << QT_ENDL;
    }

    out << "m";
    out << " g" << stitcher.gamma;
    out << " i" << (int) stitcher.interpolator;

    if (stitcher.speedUp != Stitcher::SLOW)
    {
        out << " f" << 2 - ((int) stitcher.speedUp);
    }

    out << " m" << stitcher.huberSigma;
    out << " p" << stitcher.photometricHuberSigma;
    out << stitcher.unmatchedParameters.join(QLatin1Char(' ')) << QT_ENDL;

    // Third, the images
    // Note: the order is very important here

    for (int id = 0 ; id < images.size() ; ++id)
    {
        const Image &image = images[id];

        if (image.previousComments.size() > 0)
        {
            out << image.previousComments.join(QLatin1Char('\n')) << QT_ENDL;
        }

        out << "i";
        out << " w" << image.size.width();
        out << " h" << image.size.height();
        out << " f" << (int) image.lensProjection;

        if (image.fieldOfView.referenceId >= 0 || image.fieldOfView.value > 0)
        {
            out << " v" << image.fieldOfView;
        }

        out << " Ra" << image.photometricEMoRA;
        out << " Rb" << image.photometricEMoRB;
        out << " Rc" << image.photometricEMoRC;
        out << " Rd" << image.photometricEMoRD;
        out << " Re" << image.photometricEMoRE;
        out << " Eev" << image.exposure;
        out << " Er" << image.whiteBalanceRed;
        out << " Eb" << image.whiteBalanceBlue;
        out << " r" << image.roll;
        out << " p" << image.pitch;
        out << " y" << image.yaw;
        out << " TrX" << image.mosaicCameraPositionX;
        out << " TrY" << image.mosaicCameraPositionY;
        out << " TrZ" << image.mosaicCameraPositionZ;

        if (version == V2014)
        {
            out << " Tpy" << image.mosaicProjectionPlaneYaw;
            out << " Tpp" << image.mosaicProjectionPlanePitch;
        }

        out << " j" << image.stackNumber;
        out << " a" << image.lensBarrelCoefficientA;
        out << " b" << image.lensBarrelCoefficientB;
        out << " c" << image.lensBarrelCoefficientC;
        out << " d" << image.lensCenterOffsetX;
        out << " e" << image.lensCenterOffsetY;
        out << " g" << image.lensShearX;
        out << " t" << image.lensShearY;

        const Image* imageVM = &image;

        if (image.vignettingMode.referenceId >= 0)
        {
            imageVM = &images[image.vignettingMode.referenceId];
        }

        if (((int)imageVM->vignettingMode.value) & ((int)Image::RADIAL))
        {
            out << " Va" << image.vignettingCorrectionI;
            out << " Vb" << image.vignettingCorrectionJ;
            out << " Vc" << image.vignettingCorrectionK;
            out << " Vd" << image.vignettingCorrectionL;
        }
        else
        {
            out << " Vf" << image.vignettingFlatfieldImageName;
        }

        out << " Vx" << image.vignettingOffsetX;
        out << " Vy" << image.vignettingOffsetY;
        out << " Vm" << image.vignettingMode;
        out << image.unmatchedParameters.join(QLatin1Char(' '));
        out << " n\"" << image.fileName << "\"";
        out << QT_ENDL;
    }

    // Fourth, the variable to optimize

    for (int id = 0 ; id < images.size() ; ++id)
    {
        const Image& image = images[id];

        Q_FOREACH (Optimization optim, image.optimizationParameters)
        {
            if (optim.previousComments.size() > 0)
            {
                out << optim.previousComments.join(QLatin1Char('\n')) << QT_ENDL;
            }

            out << "v ";

            switch (optim.parameter)
            {
                case Optimization::LENSA:
                    out << 'a';
                    break;

                case Optimization::LENSB:
                    out << 'b';
                    break;

                case Optimization::LENSC:
                    out << 'c';
                    break;

                case Optimization::LENSD:
                    out << 'd';
                    break;

                case Optimization::LENSE:
                    out << 'e';
                    break;

                case Optimization::LENSHFOV:
                    out << 'v';
                    break;

                case Optimization::LENSYAW:
                    out << 'y';
                    break;

                case Optimization::LENSPITCH:
                    out << 'p';
                    break;

                case Optimization::LENSROLL:
                    out << 'r';
                    break;

                case Optimization::EXPOSURE:
                    out << "Eev";
                    break;

                case Optimization::WBR:
                    out << "Er";
                    break;

                case Optimization::WBB:
                    out << "Eb";
                    break;

                case Optimization::VA:
                    out << "Va";
                    break;

                case Optimization::VB:
                    out << "Vb";
                    break;

                case Optimization::VC:
                    out << "Vc";
                    break;

                case Optimization::VD:
                    out << "Vd";
                    break;

                case Optimization::VX:
                    out << "Vx";
                    break;

                case Optimization::VY:
                    out << "Vy";
                    break;

                case Optimization::RA:
                    out << "Ra";
                    break;

                case Optimization::RB:
                    out << "Rb";
                    break;

                case Optimization::RC:
                    out << "Rc";
                    break;

                case Optimization::RD:
                    out << "Rd";
                    break;

                case Optimization::RE:
                    out << "Re";
                    break;

                case Optimization::UNKNOWN:
                    qCCritical(DIGIKAM_GENERAL_LOG) << "Unknown optimization parameter!";
                    file.close();
                    return false;
            }

            out << id << QT_ENDL;
        }
    }

    out << "v" << QT_ENDL;

    // Fifth, the masks

    for (int id = 0 ; id < images.size() ; ++id)
    {
        const Image& image = images[id];

        Q_FOREACH (Mask mask, image.masks)
        {
            if (mask.previousComments.size() > 0)
            {
                out << mask.previousComments.join(QLatin1Char('\n')) << QT_ENDL;
            }

            out << "k i" << id;
            out << " t" << (int) mask.type;
            out << " p\"";

            for (int pid = 0 ; pid < mask.hull.size() ; ++pid)
            {
                out << (pid == 0 ? "" : " ");
                out << mask.hull[pid].x() << ' ' << mask.hull[pid].y();
            }

            out << "\"" << QT_ENDL;
        }
    }

    // Sixth, the control points

    Q_FOREACH (ControlPoint cp, controlPoints)
    {
        if (cp.previousComments.size() > 0)
        {
            out << cp.previousComments.join(QLatin1Char('\n')) << QT_ENDL;
        }

        out << "c n" << cp.image1Id;
        out << " N" << cp.image2Id;
        out << " x" << cp.p1_x;
        out << " y" << cp.p1_y;
        out << " X" << cp.p2_x;
        out << " Y" << cp.p2_y;
        out << " t" << cp.type;
        out << QT_ENDL;
    }

    // Finally the ending comments

    out << lastComments.join(QLatin1Char('\n')) << QT_ENDL;

    file.close();

    return true;
}

/*
QPair<double, int> PTOType::standardDeviation(int image1Id, int image2Id)
{
    double mean_x = 0, mean_y = 0;
    double n = 0;

    Q_FOREACH (ControlPoint cp, controlPoints)
    {
        if ((cp.image1Id == image1Id && cp.image2Id == image2Id) || (cp.image1Id == image2Id && cp.image2Id == image1Id))
        {
            mean_x += cp.p2_x - cp.p1_x;
            mean_y += cp.p2_y - cp.p1_y;
            n++;
        }
    }

    if (n == 0)
    {
        return QPair<double, int>(0, 0);
    }

    mean_x /= n;
    mean_y /= n;
    double result = 0;

    Q_FOREACH (PTOType::ControlPoint cp, controlPoints)
    {
        if ((cp.image1Id == image1Id && cp.image2Id == image2Id) || (cp.image1Id == image2Id && cp.image2Id == image1Id))
        {
            double epsilon_x = (cp.p2_x - cp.p1_x) - mean_x;
            double epsilon_y = (cp.p2_y - cp.p1_y) - mean_y;
            result += epsilon_x * epsilon_x + epsilon_y * epsilon_y;
        }
    }

    return QPair<double, int>(result, n);
}

QPair<double, int> PTOType::standardDeviation(int imageId)
{
    int n = 0;
    double result = 0;

    for (int i = 0; i < images.size(); ++i)
    {
        QPair<double, int> tmp = standardDeviation(imageId, i);
        result += tmp.first;
        n += tmp.second;
    }

    return QPair<double, int>(result, n);
}

QPair<double, int> PTOType::standardDeviation()
{
    int n = 0;
    double result = 0;

    for (int i = 0; i < images.size(); ++i)
    {
        QPair<double, int> tmp = standardDeviation(i);
        result += tmp.first;
        n += tmp.second;
    }

    return QPair<double, int>(result, n);
}
*/

} // namespace Digikam
