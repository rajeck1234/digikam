/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-19
 * Description : Workflow properties dialog.
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_WORK_FLOW_DLG_H
#define DIGIKAM_BQM_WORK_FLOW_DLG_H

// Qt includes

#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QDialog>

// Locale includes

#include "workflowmanager.h"

namespace Digikam
{

class WorkflowDlg : public QDialog
{
    Q_OBJECT

public:

    explicit WorkflowDlg(const Workflow& wf, bool create = false);
    ~WorkflowDlg() override;

    QString     title()           const;
    QString     description()     const;

public:

    static bool editProps(Workflow& wf);
    static bool createNew(Workflow& wf);

private Q_SLOTS:

    void slotTitleChanged();
    void slotHelp();

private:

    // Disable
    explicit WorkflowDlg(QWidget*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BQM_WORK_FLOW_DLG_H
