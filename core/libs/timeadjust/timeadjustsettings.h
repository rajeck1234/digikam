/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-05-16
 * Description : time adjust settings widget.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TIME_ADJUST_SETTINGS_H
#define DIGIKAM_TIME_ADJUST_SETTINGS_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"
#include "timeadjustcontainer.h"

namespace Digikam
{

class TimeAdjustContainer;

class DIGIKAM_EXPORT TimeAdjustSettings : public QScrollArea
{
    Q_OBJECT

public:

    explicit TimeAdjustSettings(QWidget* const parent, bool timeAdjustTool = false);
    ~TimeAdjustSettings() override;

public:

    void setSettings(const TimeAdjustContainer& settings);
    TimeAdjustContainer settings() const;
    void detAdjustmentByClockPhotoUrl(const QUrl& url);
    void setCurrentItemUrl(const QUrl& url);

Q_SIGNALS:

    void signalSettingsChanged();
    void signalSettingsChangedTool();

private Q_SLOTS:

    void slotSrcTimestampChanged();
    void slotResetDateToCurrent();
    void slotAdjustmentTypeChanged();
    void slotDetAdjustmentByClockPhotoDialog();
    void slotDetAdjustmentByClockPhotoUrl(const QUrl& url);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TIME_ADJUST_SETTINGS_H
