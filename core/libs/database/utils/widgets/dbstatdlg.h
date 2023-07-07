/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-28
 * Description : database statistics dialog
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_STAT_DLG_H
#define DIGIKAM_DB_STAT_DLG_H

// Local includes

#include "infodlg.h"
#include "coredbconstants.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_GUI_EXPORT DBStatDlg : public InfoDlg
{
    Q_OBJECT

public:

    explicit DBStatDlg(QWidget* const parent);
    ~DBStatDlg()    override;

private Q_SLOTS:

    void slotHelp() override;

private:

    int generateItemsList(DatabaseItem::Category category, const QString& title);
};

} // namespace Digikam

#endif // DIGIKAM_DB_STAT_DLG_H
