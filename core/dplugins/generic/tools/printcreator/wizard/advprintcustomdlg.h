/* ===============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-10-01
 * Description : Dialog to allow a custom page layout
 *
 * SPDX-FileCopyrightText: 2010-2012 by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================== */

#ifndef DIGIKAM_ADV_PRINT_CUSTOM_DLG_H
#define DIGIKAM_ADV_PRINT_CUSTOM_DLG_H

// Qt includes

#include <QDialog>
#include <QWidget>

// Local includes

#include "ui_advprintcustomlayout.h"

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintCustomLayoutDlg : public QDialog,
                                public Ui::AdvPrintCustomLayout
{
    Q_OBJECT

public:

    explicit AdvPrintCustomLayoutDlg(QWidget* const parent = nullptr);
    ~AdvPrintCustomLayoutDlg() override;

    void readSettings();
    void saveSettings();

private:

    enum CustomChoice
    {
        PHOTO_GRID              = 1,
        FIT_AS_MANY_AS_POSSIBLE = 2
    };
};

} // namespace DigikamGenericPrintCreatorPlugin

#endif // DIGIKAM_ADV_PRINT_CUSTOM_DLG_H
