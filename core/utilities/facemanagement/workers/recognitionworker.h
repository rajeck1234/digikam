/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RECOGNITION_WORKER_H
#define DIGIKAM_RECOGNITION_WORKER_H

// Local includes

#include "facepipeline_p.h"
#include "faceitemretriever.h"

namespace Digikam
{

class Q_DECL_HIDDEN RecognitionWorker : public WorkerObject
{
    Q_OBJECT

public:

    explicit RecognitionWorker(FacePipeline::Private* const dd);
    ~RecognitionWorker() override;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);
    void setThreshold(double threshold, bool);

    // TODO: add set K-Nearest method

protected:

    void aboutToDeactivate() override;

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    FaceItemRetriever            imageRetriever;
    FacialRecognitionWrapper     recognizer;
    FacePipeline::Private* const d;

private:

    // Disable
    RecognitionWorker(const RecognitionWorker&)            = delete;
    RecognitionWorker& operator=(const RecognitionWorker&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_RECOGNITION_WORKER_H
