/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_HTML_PARAMETERS_PAGE_H
#define DIGIKAM_HTML_PARAMETERS_PAGE_H

// Qt includes

#include <QByteArray>
#include <QString>
#include <QWidget>

// Local includes

#include "dwizardpage.h"

using namespace Digikam;

namespace DigikamGenericHtmlGalleryPlugin
{

class GalleryInfo;

class HTMLParametersPage : public DWizardPage
{
    Q_OBJECT

public:

    explicit HTMLParametersPage(QWizard* const dialog, const QString& title);
    ~HTMLParametersPage() override;

    void initializePage() override;

    QWidget* themeParameterWidgetFromName(const QByteArray&) const;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericHtmlGalleryPlugin

#endif // DIGIKAM_HTML_PARAMETERS_PAGE_H
