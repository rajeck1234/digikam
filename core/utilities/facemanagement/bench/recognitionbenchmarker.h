/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Face recognition benchmarker
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RECOGNITION_BENCH_MARKER_H
#define DIGIKAM_RECOGNITION_BENCH_MARKER_H

// Local includes

#include "facepipeline_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN RecognitionBenchmarker : public WorkerObject
{
    Q_OBJECT

public:

    explicit RecognitionBenchmarker(FacePipeline::Private* const dd);
    QString result() const;

public Q_SLOTS:

    void process(FacePipelineExtendedPackage::Ptr package);

Q_SIGNALS:

    void processed(FacePipelineExtendedPackage::Ptr package);

protected:

    class Q_DECL_HIDDEN Statistics
    {
    public:

        Statistics();

    public:

        int knownFaces;
        int correctlyRecognized;
    };

protected:

    QMap<int, Statistics>        results;

    FacePipeline::Private* const d;
    FacialRecognitionWrapper     recognizer;
};

} // namespace Digikam

#endif // DIGIKAM_RECOGNITION_BENCH_MARKER_H
