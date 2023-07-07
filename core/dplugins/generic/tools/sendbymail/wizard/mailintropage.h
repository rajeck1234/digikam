/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items by email.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MAIL_INTRO_PAGE_H
#define DIGIKAM_MAIL_INTRO_PAGE_H

// Qt includes

#include <QString>

// Local includes

#include "dwizardpage.h"

using namespace Digikam;

namespace DigikamGenericSendByMailPlugin
{

class MailIntroPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit MailIntroPage(QWizard* const dialog, const QString& title);
    ~MailIntroPage() override;

    void initializePage() override;
    bool validatePage() override;
    bool isComplete() const override;

private Q_SLOTS:

    void slotBinariesFound();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericSendByMailPlugin

#endif // DIGIKAM_MAIL_INTRO_PAGE_H
