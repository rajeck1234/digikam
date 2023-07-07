/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-23
 * Description : file action progress indicator
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FILE_ACTION_PROGRESS_H
#define DIGIKAM_FILE_ACTION_PROGRESS_H

// Qt includes

#include <QString>

// Local includes

#include "progressmanager.h"

namespace Digikam
{

class FileActionProgress : public ProgressItem
{
    Q_OBJECT

public:

    explicit FileActionProgress(const QString& name);
    ~FileActionProgress() override;

Q_SIGNALS:

    void signalComplete();

private Q_SLOTS:

    void slotProgressValue(float);
    void slotProgressStatus(const QString&);
    void slotCompleted();
    void slotCancel();

private:

    // Disable
    FileActionProgress(QObject*);
};

} // namespace Digikam

#endif // DIGIKAM_FILE_ACTION_PROGRESS_H
