/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-01-24
 * Description : a progress bar used to display action
 *               progress or a text in status bar.
 *               Progress events are dispatched to ProgressManager.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_STATUS_PROGRESS_BAR_H
#define DIGIKAM_STATUS_PROGRESS_BAR_H

// Qt includes

#include <QStackedWidget>
#include <QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{
class ProgressItem;

class DIGIKAM_EXPORT StatusProgressBar : public QStackedWidget
{
    Q_OBJECT

public:

    enum StatusProgressBarMode
    {
        TextMode = 0,
        ProgressBarMode,
        CancelProgressBarMode
    };

public:

    explicit StatusProgressBar(QWidget* const parent = nullptr);
    ~StatusProgressBar() override;

    void setAlignment(Qt::Alignment a);

    void setProgressBarMode(int mode, const QString& text = QString());

    int  progressValue()      const;

    int  progressTotalSteps() const;
    void setProgressTotalSteps(int v);

    void setNotify(bool b);
    void setNotificationTitle(const QString& title, const QIcon& icon);

public Q_SLOTS:

    void setText(const QString& text);
    void setProgressValue(int v);
    void setProgressText(const QString& text);

Q_SIGNALS:

    void signalCancelButtonPressed();

private:

    ProgressItem* currentProgressItem() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_STATUS_PROGRESS_BAR_H
