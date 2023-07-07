/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for finger-prints generator.
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FINGERPRINTS_TASK_H
#define DIGIKAM_FINGERPRINTS_TASK_H

// Qt includes

#include <QPixmap>
#include <QThread>

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{

class LoadingDescription;
class MaintenanceData;
class DImg;

class FingerprintsTask : public ActionJob
{
    Q_OBJECT

public:

    explicit FingerprintsTask();
    ~FingerprintsTask() override;

    void setMaintenanceData(MaintenanceData* const data = nullptr);

Q_SIGNALS:

    void signalFinished(const QImage&);

protected:

    void run() override;

private:

    // Disable
    FingerprintsTask(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FINGERPRINTS_TASK_H
