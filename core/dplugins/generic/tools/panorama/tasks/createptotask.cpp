/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "createptotask.h"
#include "ptotype.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericPanoramaPlugin
{

CreatePtoTask::CreatePtoTask(const QString& workDirPath,
                             PanoramaFileType fileType,
                             QUrl& ptoUrl,
                             const QList<QUrl>& inputFiles,
                             const PanoramaItemUrlsMap& preProcessedMap,
                             bool addGPlusMetadata,
                             const QString& huginVersion)
    : PanoTask(PANO_CREATEPTO,
      workDirPath),
      ptoUrl(ptoUrl),
      preProcessedMap(&preProcessedMap),
      fileType(addGPlusMetadata ? JPEG : fileType),
      inputFiles(inputFiles),
      addGPlusMetadata(addGPlusMetadata),
      huginVersion(huginVersion)
{
}

CreatePtoTask::~CreatePtoTask()
{
}

void CreatePtoTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    ptoUrl = tmpDir;
    ptoUrl.setPath(ptoUrl.path() + QLatin1String("pano_base.pto"));

    QFile pto(ptoUrl.toLocalFile());

    if (pto.exists())
    {
        errString   = i18n("PTO file already created in the temporary directory.");
        successFlag = false;

        return;
    }

    if (!pto.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text))
    {
        errString   = i18n("PTO file cannot be created in the temporary directory.");
        successFlag = false;

        return;
    }

    pto.close();

    // 1. Project parameters

    PTOType panoBase(huginVersion);

    if (addGPlusMetadata)
    {
        panoBase.project.projection = PTOType::Project::EQUIRECTANGULAR;
    }
    else
    {
        panoBase.project.projection = PTOType::Project::CYLINDRICAL;
    }

    panoBase.project.fieldOfView = 0;
    panoBase.project.hdr         = false;

    switch (fileType)
    {
        case JPEG:
            panoBase.project.fileFormat.fileType = PTOType::Project::FileFormat::JPEG;
            panoBase.project.fileFormat.quality = 90;
            break;

        case TIFF:
            panoBase.project.fileFormat.fileType = PTOType::Project::FileFormat::TIFF_m;
            panoBase.project.fileFormat.compressionMethod = PTOType::Project::FileFormat::LZW;
            panoBase.project.fileFormat.savePositions = false;
            panoBase.project.fileFormat.cropped = false;
            break;

        case HDR:
            panoBase.project.hdr = true;
            // TODO HDR
            break;
    }
/*
    panoBase.project.bitDepth = PTOType::Project::FLOAT;
    panoBase.project.crop.setLeft(X_left);
    panoBase.project.crop.setRight(X_right);
    panoBase.project.crop.setTop(X_top);
    panoBase.project.crop.setBottom(X_bottom);
*/
    panoBase.project.photometricReferenceId = 0;

    // 2. Images

    panoBase.images.reserve(inputFiles.size());
    panoBase.images.resize(inputFiles.size());
    int i = 0;

    for (i = 0 ; i < inputFiles.size() ; ++i)
    {
        QUrl inputFile(inputFiles.at(i));
        QUrl preprocessedUrl(preProcessedMap->value(inputFile).preprocessedUrl);
        m_meta.load(preprocessedUrl.toLocalFile());
        QSize size                        = m_meta.getPixelSize();

        panoBase.images[i]                = PTOType::Image();
        panoBase.images[i].lensProjection = PTOType::Image::RECTILINEAR;
        panoBase.images[i].size           = size;

        if (i > 0)
        {
            // We suppose that the pictures are all taken with the same camera and lens

            panoBase.images[i].lensBarrelCoefficientA.referenceId = 0;
            panoBase.images[i].lensBarrelCoefficientB.referenceId = 0;
            panoBase.images[i].lensBarrelCoefficientC.referenceId = 0;
            panoBase.images[i].lensCenterOffsetX.referenceId      = 0;
            panoBase.images[i].lensCenterOffsetY.referenceId      = 0;
            panoBase.images[i].lensShearX.referenceId             = 0;
            panoBase.images[i].lensShearY.referenceId             = 0;
            panoBase.images[i].vignettingCorrectionI.referenceId  = 0;
            panoBase.images[i].vignettingCorrectionJ.referenceId  = 0;
            panoBase.images[i].vignettingCorrectionK.referenceId  = 0;
            panoBase.images[i].vignettingCorrectionL.referenceId  = 0;
            panoBase.images[i].vignettingOffsetX.referenceId      = 0;
            panoBase.images[i].vignettingOffsetY.referenceId      = 0;
        }
        else
        {
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::LENSA;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::LENSB;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::LENSC;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::LENSD;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::LENSE;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::VA;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::VB;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::VC;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::VD;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::VX;
            panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
            panoBase.images[i].optimizationParameters.last().parameter = PTOType::Optimization::VY;
        }

        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::RA;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::RB;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::RC;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::RD;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::RE;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::EXPOSURE;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::WBR;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::WBB;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::LENSYAW;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::LENSPITCH;
        panoBase.images[i].optimizationParameters.append(PTOType::Optimization());
        panoBase.images[i].optimizationParameters.last().parameter =  PTOType::Optimization::LENSROLL;
        panoBase.images[i].fileName = preprocessedUrl.toLocalFile();
    }

    switch (fileType)
    {
        case TIFF:
            panoBase.lastComments << QLatin1String("#hugin_outputImageType tif");
            panoBase.lastComments << QLatin1String("#hugin_outputImageTypeCompression LZW");
            break;

        case JPEG:
            panoBase.lastComments << QLatin1String("#hugin_outputImageType jpg");
            panoBase.lastComments << QLatin1String("#hugin_outputJPEGQuality 90");
            break;

        case HDR:
            // TODO: HDR
            break;
    }

    panoBase.createFile(ptoUrl.toLocalFile());

    successFlag = true;
}

} // namespace DigikamGenericPanoramaPlugin
