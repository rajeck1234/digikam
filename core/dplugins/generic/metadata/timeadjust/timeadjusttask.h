/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-31
 * Description : time adjust actions using threads.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TIME_ADJUST_TASK_H
#define DIGIKAM_TIME_ADJUST_TASK_H

// Qt includes

#include <QUrl>

// Local includes

#include "timeadjustthread.h"
#include "actionthreadbase.h"
#include "timeadjustcontainer.h"

using namespace Digikam;

namespace DigikamGenericTimeAdjustPlugin
{

class TimeAdjustTask : public ActionJob
{
    Q_OBJECT

public:

    explicit TimeAdjustTask(const QUrl& url, TimeAdjustThread* const thread);
    ~TimeAdjustTask() override;

    void setSettings(const TimeAdjustContainer& settings);

Q_SIGNALS:

    void signalProcessStarted(const QUrl&);
    void signalDateTimeForUrl(const QUrl&, const QDateTime&, bool);
    void signalProcessEnded(const QUrl&, const QDateTime&, const QDateTime&, int);

protected:

    void run() override;

private:

    // Disable
    TimeAdjustTask(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

// ------------------------------------------------------------------

class TimePreviewTask : public ActionJob
{
    Q_OBJECT

public:

    explicit TimePreviewTask(const QUrl& url, TimeAdjustThread* const thread);
    ~TimePreviewTask() override;

    void setSettings(const TimeAdjustContainer& settings);

Q_SIGNALS:

    void signalPreviewReady(const QUrl&, const QDateTime&, const QDateTime&);

protected:

    void run() override;

private:

    // Disable
    TimePreviewTask(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericTimeAdjustPlugin

#endif // DIGIKAM_TIME_ADJUST_TASK
