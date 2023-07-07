/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-03-15
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_OPTIMISATION_TASK_H
#define DIGIKAM_OPTIMISATION_TASK_H

// Local includes

#include "commandtask.h"

namespace DigikamGenericPanoramaPlugin
{

class OptimisationTask : public CommandTask
{
public:

    explicit OptimisationTask(const QString& workDirPath,
                              const QUrl& input,
                              QUrl& autoOptimiserPtoUrl,
                              bool  levelHorizon,
                              bool  gPano,
                              const QString& autooptimiserPath);
    ~OptimisationTask() override;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    QUrl&       autoOptimiserPtoUrl;
    const QUrl& ptoUrl;
    const bool  levelHorizon;
    const bool  buildGPano;

private:

    // Disable
    OptimisationTask() = delete;

    Q_DISABLE_COPY(OptimisationTask)
};

} // namespace DigikamGenericPanoramaPlugin

#endif // DIGIKAM_OPTIMISATION_TASK_H
