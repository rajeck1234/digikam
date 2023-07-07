/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TOOL_TIPS_PAGE_H
#define DIGIKAM_TOOL_TIPS_PAGE_H

// Local includes

#include "dwizardpage.h"

namespace Digikam
{

class TooltipsPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit TooltipsPage(QWizard* const dlg);
    ~TooltipsPage() override;

    void saveSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TOOL_TIPS_PAGE_H
