/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-14
 * Description : Thread actions task for thumbs generator.
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_THUMBS_TASK_H
#define DIGIKAM_THUMBS_TASK_H

// Qt includes

#include <QImage>

// Local includes

#include "actionthreadbase.h"

namespace Digikam
{

class LoadingDescription;
class MaintenanceData;

class ThumbsTask : public ActionJob
{
    Q_OBJECT

public:

    explicit ThumbsTask();
    ~ThumbsTask() override;

    void setMaintenanceData(MaintenanceData* const data = nullptr);

Q_SIGNALS:

    void signalFinished(const QImage&);

protected:

    void run() override;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                             const QImage& image);

private:

    // Disable
    ThumbsTask(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_THUMBS_TASK_H
