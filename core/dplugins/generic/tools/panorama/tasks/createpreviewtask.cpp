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

#include "createpreviewtask.h"

// Qt includes

#include <QFile>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericPanoramaPlugin
{

CreatePreviewTask::CreatePreviewTask(const QString& workDirPath,
                                     QSharedPointer<const PTOType> inputPTO,
                                     QUrl& previewPtoUrl,
                                     const PanoramaItemUrlsMap& preProcessedUrlsMap)
    : PanoTask(PANO_CREATEMKPREVIEW, workDirPath),
      previewPtoUrl(previewPtoUrl),
      ptoData(inputPTO),
      preProcessedUrlsMap(preProcessedUrlsMap)
{
}

CreatePreviewTask::~CreatePreviewTask()
{
}

void CreatePreviewTask::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
    PTOType data(*ptoData);

    if (data.images.size() != preProcessedUrlsMap.size())
    {
        errString   = i18n("Project file parsing failed.");
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Missing parsing data!";
        successFlag = false;
        return;
    }

    m_meta.load(preProcessedUrlsMap.begin().value().preprocessedUrl.toLocalFile());
    double wIn                       = (double)m_meta.getPixelSize().width();

    m_meta.load(preProcessedUrlsMap.begin().value().previewUrl.toLocalFile());
    double wOut                      = (double)m_meta.getPixelSize().width();

    double scalingFactor             = wOut / wIn;

    data.project.fileFormat.fileType = PTOType::Project::FileFormat::JPEG;
    data.project.fileFormat.quality  = 90;
    data.project.size.setHeight(data.project.size.height() * scalingFactor);
    data.project.size.setWidth(data.project.size.width() * scalingFactor);
    data.project.crop                = QRect();

    for (auto& image : data.images)
    {
        QUrl imgUrl                           = QUrl::fromLocalFile(image.fileName);

        PanoramaItemUrlsMap::const_iterator it;
        const PanoramaItemUrlsMap* const ppum = &preProcessedUrlsMap;

        for (it = ppum->constBegin() ;
             it != ppum->constEnd() && (it.value().preprocessedUrl.toLocalFile() != imgUrl.toLocalFile()) ;
             ++it);

        if (it == ppum->constEnd())
        {
            errString   = i18n("Unknown input file in the project file: <filename>%1</filename>", image.fileName);
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Unknown input File in the PTO: " << image.fileName;
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "IMG: " << imgUrl.toLocalFile();
            successFlag = false;

            return;
        }

        image.fileName = it.value().previewUrl.toLocalFile();
        m_meta.load(image.fileName);
        image.size     = m_meta.getPixelSize();
        image.optimizationParameters.clear();
    }

    // Remove unnecessary stuff

    data.controlPoints.clear();

    // Add two commented line for a JPEG output

    data.lastComments.clear();
    data.lastComments << QLatin1String("#hugin_outputImageType jpg");
    data.lastComments << QLatin1String("#hugin_outputJPEGQuality 90");

    previewPtoUrl = tmpDir;
    previewPtoUrl.setPath(previewPtoUrl.path() + QLatin1String("preview.pto"));
    data.createFile(previewPtoUrl.toLocalFile());

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "Preview PTO File created: " << previewPtoUrl.fileName();

    successFlag = true;
}

} // namespace DigikamGenericPanoramaPlugin
