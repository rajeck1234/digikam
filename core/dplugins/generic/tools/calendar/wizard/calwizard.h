/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : a tool to create calendar.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAL_WIZARD_H
#define DIGIKAM_CAL_WIZARD_H

// Qt includes

#include <QMap>

// Local includes

#include "dwizarddlg.h"
#include "dwizardpage.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericCalendarPlugin
{

class CalWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit CalWizard(QWidget* const parent, DInfoInterface* const iface = nullptr);
    ~CalWizard() override;

    DInfoInterface* iface() const;

private:

    void print();

private Q_SLOTS:

    void slotPageSelected(int current);
    void printComplete();
    void updatePage(int page);

private:

    class Private;
    Private* const d;
};

} // Namespace Digikam

#endif // DIGIKAM_CAL_WIZARD_H
