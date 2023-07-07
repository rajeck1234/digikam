/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ADV_PRINT_CROP_PAGE_H
#define DIGIKAM_ADV_PRINT_CROP_PAGE_H

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "dwizardpage.h"
#include "ditemslist.h"
#include "ui_advprintcroppage.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintCropPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintCropPage(QWizard* const wizard, const QString& title);
    ~AdvPrintCropPage() override;

    Ui_AdvPrintCropPage* ui() const;

    void setBtnCropEnabled();

    void initializePage() override;
    bool validatePage() override;

private Q_SLOTS:

    void slotCropSelection(int);
    void slotBtnCropPrevClicked();
    void slotBtnCropNextClicked();
    void slotBtnCropRotateLeftClicked();
    void slotBtnCropRotateRightClicked();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericPrintCreatorPlugin

#endif // DIGIKAM_ADV_PRINT_CROP_PAGE_H
