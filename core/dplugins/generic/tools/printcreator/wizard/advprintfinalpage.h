/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ADV_PRINT_FINAL_PAGE_H
#define DIGIKAM_ADV_PRINT_FINAL_PAGE_H

// Qt includes

#include <QString>
#include <QPainter>
#include <QList>
#include <QRect>
#include <QStringList>

// Local includes

#include "dwizardpage.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintPhoto;
class AdvPrintPhotoSize;
class AdvPrintPhotoPage;

class AdvPrintFinalPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit AdvPrintFinalPage(QWizard* const dialog, const QString& title);
    ~AdvPrintFinalPage() override;

    void initializePage() override;
    bool isComplete() const override;
    void cleanupPage() override;

    bool checkTempPath(const QString& tempPath) const;
    void removeGimpFiles();
    void setPhotoPage(AdvPrintPhotoPage* const photoPage);

private Q_SLOTS:

    void slotProcess();
    void slotPrint(bool);
    void slotDone(bool);
    void slotMessage(const QString&, bool);

private:

    bool print();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericPrintCreatorPlugin

#endif // DIGIKAM_ADV_PRINT_FINAL_PAGE_H
