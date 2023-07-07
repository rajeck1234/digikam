/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-05
 * Description : a tool to create panorama by fusion of several images.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CREATE_FINAL_PTO_TASK_H
#define DIGIKAM_CREATE_FINAL_PTO_TASK_H

// Qt includes

#include <QSharedPointer>

// Local includes

#include "panotask.h"
#include "ptotype.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CreateFinalPtoTask : public PanoTask
{
public:

    explicit CreateFinalPtoTask(const QString& workDirPath,
                                QSharedPointer<const PTOType> ptoData,
                                QUrl& finalPtoUrl,
                                const QRect& crop);
    ~CreateFinalPtoTask() override;

protected:

    void run(ThreadWeaver::JobPointer self, ThreadWeaver::Thread* thread) override;

private:

    PTOType     ptoData;
    QUrl&       finalPtoUrl;
    const QRect crop;

private:

    // Disable
    CreateFinalPtoTask() = delete;

    Q_DISABLE_COPY(CreateFinalPtoTask)
};

} // namespace DigikamGenericPanoramaPlugin

#endif // DIGIKAM_CREATE_FINAL_PTO_TASK_H
