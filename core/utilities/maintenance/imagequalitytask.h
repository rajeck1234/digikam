/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-19
 * Description : Thread actions task for image quality sorter.
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_QUALITY_TASK_H
#define DIGIKAM_IMAGE_QUALITY_TASK_H

// Qt includes

#include <QImage>

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{

class ImageQualityContainer;
class MaintenanceData;

class ImageQualityTask : public ActionJob
{
    Q_OBJECT

public:

    explicit ImageQualityTask();
    ~ImageQualityTask()     override;

    void setQuality(const ImageQualityContainer& quality);
    void setMaintenanceData(MaintenanceData* const data = nullptr);

Q_SIGNALS:

    void signalFinished(const QImage&);

public Q_SLOTS:

    void slotCancel();

protected:

    void run()              override;

private:

    // Disable
    ImageQualityTask(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMAGE_QUALITY_TASK_H
