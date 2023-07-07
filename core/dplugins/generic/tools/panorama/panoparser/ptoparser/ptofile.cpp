/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-04
 * Description : a tool to create panorama by fusion of several images.
 *               This parser is based on pto file format described here:
 *               hugin.sourceforge.net/docs/nona/nona.txt, and
 *               on pto files produced by Hugin's tools.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ptofile.h"

// C++ includes

#include <string>
#include <iostream>

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"

extern "C"
{
#include "tparser.h"
#include "tparsergetters.h"
}

namespace Digikam
{

class Q_DECL_HIDDEN PTOFile::Private
{
public:

    explicit Private(const QString& huginVersion)
      : script(nullptr),
        huginVersion(huginVersion)
    {
    }

    pt_script*     script;
    const QString& huginVersion;
};

PTOFile::PTOFile(const QString& huginVersion)
    : d(new Private(huginVersion))
{
}

PTOFile::~PTOFile()
{
    if (d->script != nullptr)
    {
        panoScriptFree(d->script);
        delete d->script;
    }

    delete d;
}

bool PTOFile::openFile(const QString& path)
{
/*
    mtrace();
*/
    if (d->script != nullptr)
    {
        panoScriptFree(d->script);
        delete d->script;
        d->script = nullptr;
    }

    d->script = new pt_script();

    if (!panoScriptParse(QFile::encodeName(path).constData(), d->script))
    {
        return false;
    }
/*
    muntrace();
*/
    return true;
}

PTOType* PTOFile::getPTO()
{
    if (d->script == nullptr)
    {
        return nullptr;
    }

    PTOType* const out = new PTOType(d->huginVersion);

    // Project data conversion

    for (int c = 0 ; c < panoScriptGetPanoPrevCommentsCount(d->script) ; ++c)
    {
        out->project.previousComments << QString::fromLocal8Bit(panoScriptGetPanoComment(d->script, c));
    }

    out->project.size.setHeight(panoScriptGetPanoHeight(d->script));
    out->project.size.setWidth(panoScriptGetPanoWidth(d->script));
    out->project.crop.setLeft(panoScriptGetPanoCropLeft(d->script));
    out->project.crop.setRight(panoScriptGetPanoCropRight(d->script));
    out->project.crop.setTop(panoScriptGetPanoCropTop(d->script));
    out->project.crop.setBottom(panoScriptGetPanoCropBottom(d->script));
    out->project.projection          = PTOType::Project::ProjectionType(panoScriptGetPanoProjection(d->script));
    out->project.fieldOfView         = panoScriptGetPanoHFOV(d->script);
    out->project.fileFormat.fileType = PTOType::Project::FileFormat::FileType(panoScriptGetPanoOutputFormat(d->script));

    switch (out->project.fileFormat.fileType)
    {
        case PTOType::Project::FileFormat::TIFF_m:
        case PTOType::Project::FileFormat::TIFF_multilayer:
        {
            out->project.fileFormat.savePositions = panoScriptGetPanoOutputSaveCoordinates(d->script) != 0;
            out->project.fileFormat.cropped       = panoScriptGetPanoOutputCropped(d->script) != 0;
            break;
        }

        case PTOType::Project::FileFormat::TIFF:
        {
            int method = panoScriptGetPanoOutputCompression(d->script);

            if (method != -1)
                out->project.fileFormat.compressionMethod = PTOType::Project::FileFormat::CompressionMethod(method);

            break;
        }

        case PTOType::Project::FileFormat::JPEG:
        {
            int quality = panoScriptGetPanoOutputQuality(d->script);

            if (quality >= 0)
                out->project.fileFormat.quality = quality;

            break;
        }

        default:
        {
            break;
        }
    }

    out->project.exposure               = panoScriptGetPanoExposure(d->script);
    out->project.hdr                    = panoScriptGetPanoIsHDR(d->script) != 0;
    out->project.bitDepth               = PTOType::Project::BitDepth(panoScriptGetPanoBitDepth(d->script));
    out->project.photometricReferenceId = panoScriptGetPanoImageReference(d->script);

    // NOTE: there should not be any unmatched parameters at this point, because the parsing would otherwise have failed

    // Stitcher

    for (int c = 0 ; c < panoScriptGetOptimizePrevCommentsCount(d->script) ; ++c)
    {
        out->stitcher.previousComments << QString::fromLocal8Bit(panoScriptGetOptimizeComment(d->script, c));
    }

    out->stitcher.gamma                 = panoScriptGetOptimizeGamma(d->script);
    out->stitcher.interpolator          = PTOType::Stitcher::Interpolator(panoScriptGetOptimizeInterpolator(d->script));
    out->stitcher.speedUp               = PTOType::Stitcher::SpeedUp(panoScriptGetOptimizeSpeedUp(d->script));
    out->stitcher.huberSigma            = panoScriptGetOptimizeHuberSigma(d->script);
    out->stitcher.photometricHuberSigma = panoScriptGetOptimizePhotometricHuberSigma(d->script);

    // Images

    out->images.clear();

    for (int i = 0 ; i < panoScriptGetImagesCount(d->script) ; ++i)
    {
        out->images.insert(i, PTOType::Image());
        PTOType::Image& image = out->images.last();

        int tmpRef;

        for (int c = 0 ; c < panoScriptGetImagePrevCommentsCount(d->script, i) ; ++c)
        {
            image.previousComments << QString::fromLocal8Bit(panoScriptGetImageComment(d->script, i, c));
        }

        image.size.setWidth(panoScriptGetImageWidth(d->script, i));
        image.size.setHeight(panoScriptGetImageHeight(d->script, i));
        image.id             = i;
        image.lensProjection = PTOType::Image::LensProjection(panoScriptGetImageProjection(d->script, i));
        tmpRef               = panoScriptGetImageHFOVRef(d->script, i);

        if (tmpRef == -1)
        {
            image.fieldOfView.value = panoScriptGetImageHFOV(d->script, i);
        }
        else
        {
            image.fieldOfView.referenceId = tmpRef;
        }

        image.yaw   = panoScriptGetImageYaw(d->script, i);
        image.pitch = panoScriptGetImagePitch(d->script, i);
        image.roll  = panoScriptGetImageRoll(d->script, i);
        tmpRef      = panoScriptGetImageCoefARef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensBarrelCoefficientA.value = panoScriptGetImageCoefA(d->script, i);
        }
        else
        {
            image.lensBarrelCoefficientA.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageCoefBRef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensBarrelCoefficientB.value = panoScriptGetImageCoefB(d->script, i);
        }
        else
        {
            image.lensBarrelCoefficientB.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageCoefCRef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensBarrelCoefficientC.value = panoScriptGetImageCoefC(d->script, i);
        }
        else
        {
            image.lensBarrelCoefficientC.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageCoefDRef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensCenterOffsetX.value = panoScriptGetImageCoefD(d->script, i);
        }
        else
        {
            image.lensCenterOffsetX.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageCoefERef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensCenterOffsetY.value = panoScriptGetImageCoefE(d->script, i);
        }
        else
        {
            image.lensCenterOffsetY.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageSheerXRef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensShearX.value = panoScriptGetImageSheerX(d->script, i);
        }
        else
        {
            image.lensShearX.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageSheerYRef(d->script, i);

        if (tmpRef == -1)
        {
            image.lensShearY.value = panoScriptGetImageSheerY(d->script, i);
        }
        else
        {
            image.lensShearY.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageExposureRef(d->script, i);

        if (tmpRef == -1)
        {
            image.exposure.value = panoScriptGetImageExposure(d->script, i);
        }
        else
        {
            image.exposure.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageWBRedRef(d->script, i);

        if (tmpRef == -1)
        {
            image.whiteBalanceRed.value = panoScriptGetImageWBRed(d->script, i);
        }
        else
        {
            image.whiteBalanceRed.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageWBBlueRef(d->script, i);

        if (tmpRef == -1)
        {
            image.whiteBalanceBlue.value = panoScriptGetImageWBBlue(d->script, i);
        }
        else
        {
            image.whiteBalanceBlue.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingModeRef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingMode.value = PTOType::Image::VignettingMode(panoScriptGetImageVignettingMode(d->script, i));
        }
        else
        {
            image.vignettingMode.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingCoeffARef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingCorrectionI.value = panoScriptGetImageVignettingCoeffA(d->script, i);
        }
        else
        {
            image.vignettingCorrectionI.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingCoeffBRef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingCorrectionJ.value = panoScriptGetImageVignettingCoeffB(d->script, i);
        }
        else
        {
            image.vignettingCorrectionJ.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingCoeffCRef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingCorrectionK.value = panoScriptGetImageVignettingCoeffC(d->script, i);
        }
        else
        {
            image.vignettingCorrectionK.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingCoeffDRef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingCorrectionL.value = panoScriptGetImageVignettingCoeffD(d->script, i);
        }
        else
        {
            image.vignettingCorrectionL.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingCoeffXRef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingOffsetX.value = panoScriptGetImageVignettingCoeffX(d->script, i);
        }
        else
        {
            image.vignettingOffsetX.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImageVignettingCoeffYRef(d->script, i);

        if (tmpRef == -1)
        {
            image.vignettingOffsetY.value = panoScriptGetImageVignettingCoeffY(d->script, i);
        }
        else
        {
            image.vignettingOffsetY.referenceId = tmpRef;
        }

        char* const flatfield = panoScriptGetImageVignettingFlatField(d->script, i);

        if (flatfield != nullptr)
        {
            image.vignettingFlatfieldImageName = QString::fromLocal8Bit(flatfield);
        }

        tmpRef = panoScriptGetImagePhotometricCoeffARef(d->script, i);

        if (tmpRef == -1)
        {
            image.photometricEMoRA.value = panoScriptGetImagePhotometricCoeffA(d->script, i);
        }
        else
        {
            image.photometricEMoRA.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImagePhotometricCoeffBRef(d->script, i);

        if (tmpRef == -1)
        {
            image.photometricEMoRB.value = panoScriptGetImagePhotometricCoeffB(d->script, i);
        }
        else
        {
            image.photometricEMoRB.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImagePhotometricCoeffCRef(d->script, i);

        if (tmpRef == -1)
        {
            image.photometricEMoRC.value = panoScriptGetImagePhotometricCoeffC(d->script, i);
        }
        else
        {
            image.photometricEMoRC.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImagePhotometricCoeffDRef(d->script, i);

        if (tmpRef == -1)
        {
            image.photometricEMoRD.value = panoScriptGetImagePhotometricCoeffD(d->script, i);
        }
        else
        {
            image.photometricEMoRD.referenceId = tmpRef;
        }

        tmpRef = panoScriptGetImagePhotometricCoeffERef(d->script, i);

        if (tmpRef == -1)
        {
            image.photometricEMoRE.value = panoScriptGetImagePhotometricCoeffE(d->script, i);
        }
        else
        {
            image.photometricEMoRE.referenceId = tmpRef;
        }

        image.mosaicCameraPositionX         = panoScriptGetImageCameraTranslationX(d->script, i);
        image.mosaicCameraPositionY         = panoScriptGetImageCameraTranslationY(d->script, i);
        image.mosaicCameraPositionZ         = panoScriptGetImageCameraTranslationZ(d->script, i);
        image.mosaicProjectionPlaneYaw      = panoScriptGetImageProjectionPlaneYaw(d->script, i);
        image.mosaicProjectionPlanePitch    = panoScriptGetImageProjectionPlanePitch(d->script, i);
        image.crop.setLeft(panoScriptGetImageCropLeft(d->script, i));
        image.crop.setRight(panoScriptGetImageCropRight(d->script, i));
        image.crop.setTop(panoScriptGetImageCropTop(d->script, i));
        image.crop.setBottom(panoScriptGetImageCropBottom(d->script, i));
        tmpRef = panoScriptGetImageStackRef(d->script, i);

        if (tmpRef == -1)
        {
            image.stackNumber.value = panoScriptGetImageStack(d->script, i);
        }
        else
        {
            image.stackNumber.referenceId = tmpRef;
        }

        image.fileName = QString::fromLocal8Bit(panoScriptGetImageName(d->script, i));
    }

    // Masks

    for (int m = 0 ; m < panoScriptGetMaskCount(d->script) ; ++m)
    {
        int image           = panoScriptGetMaskImage(d->script, m);
        out->images[image].masks.append(PTOType::Mask());
        PTOType::Mask& mask = out->images[image].masks.last();

        for (int c = 0 ; c < panoScriptGetMaskPrevCommentCount(d->script, m) ; ++c)
        {
            mask.previousComments << QString::fromLocal8Bit(panoScriptGetMaskComment(d->script, m, c));
        }

        mask.type = PTOType::Mask::MaskType(panoScriptGetMaskType(d->script, m));

        for (int pt = 0 ; pt < panoScriptGetMaskPointCount(d->script, m) ; ++pt)
        {
            int x = panoScriptGetMaskPointX(d->script, m, pt);
            int y = panoScriptGetMaskPointY(d->script, m, pt);
            mask.hull.append(QPoint(x, y));
        }
    }

    // Variable optimization

    for (int v = 0 ; v < panoScriptGetVarsToOptimizeCount(d->script) ; ++v)
    {
        int image                = panoScriptGetVarsToOptimizeImageId(d->script, v);
        out->images[image].optimizationParameters.append(PTOType::Optimization());
        PTOType::Optimization& o = out->images[image].optimizationParameters.last();

        for (int c = 0 ; c < panoScriptGetVarsToOptimizePrevCommentCount(d->script, v) ; ++c)
        {
            o.previousComments << QString::fromLocal8Bit(panoScriptGetVarsToOptimizeComment(d->script, v, c));
        }

        o.parameter = PTOType::Optimization::Parameter(panoScriptGetVarsToOptimizeName(d->script, v));
    }

    // Control Points

    for (int cp = 0 ; cp < panoScriptGetCtrlPointCount(d->script) ; ++cp)
    {
        out->controlPoints.append(PTOType::ControlPoint());
        PTOType::ControlPoint& ctrlPoint = out->controlPoints.last();

        for (int c = 0 ; c < panoScriptGetCtrlPointPrevCommentCount(d->script, cp) ; ++c)
        {
            ctrlPoint.previousComments << QString::fromLocal8Bit(panoScriptGetCtrlPointComment(d->script, cp, c));
        }

        ctrlPoint.image1Id = panoScriptGetCtrlPointImage1(d->script, cp);
        ctrlPoint.image2Id = panoScriptGetCtrlPointImage2(d->script, cp);
        ctrlPoint.p1_x     = panoScriptGetCtrlPointX1(d->script,     cp);
        ctrlPoint.p1_y     = panoScriptGetCtrlPointY1(d->script,     cp);
        ctrlPoint.p2_x     = panoScriptGetCtrlPointX2(d->script,     cp);
        ctrlPoint.p2_y     = panoScriptGetCtrlPointY2(d->script,     cp);
    }

    // Ending comments

    for (int c = 0 ; c < panoScriptGetEndingCommentCount(d->script) ; ++c)
    {
        out->lastComments << QString::fromLocal8Bit(panoScriptGetEndingComment(d->script, c));
    }

    return out;
}

} // namespace Digikam
