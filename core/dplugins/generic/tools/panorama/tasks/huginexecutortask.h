/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-11-04
 * Description : interface to hugin_executor
 *
 * SPDX-FileCopyrightText: 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_HUGIN_EXECUTOR_TASK_H
#define DIGIKAM_HUGIN_EXECUTOR_TASK_H

// Local includes

#include "commandtask.h"

namespace DigikamGenericPanoramaPlugin
{

class HuginExecutorTask : public CommandTask
{
public:

    explicit HuginExecutorTask(const QString& workDirPath,
                               const QUrl& input,
                               QUrl& panoUrl,
                               PanoramaFileType fileType,
                               const QString& huginExecutorPath,
                               bool preview);
    ~HuginExecutorTask() override;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    const QUrl&            ptoUrl;
    QUrl&                  panoUrl;
    const PanoramaFileType fileType;

private:

    // Disable
    HuginExecutorTask() = delete;

    Q_DISABLE_COPY(HuginExecutorTask)
};

} // namespace DigikamGenericPanoramaPlugin

#endif // DIGIKAM_HUGIN_EXECUTOR_TASK_H
