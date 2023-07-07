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

#ifndef DIGIKAM_DATABASE_PAGE_H
#define DIGIKAM_DATABASE_PAGE_H

// Qt includes

#include <QString>
#include <QUrl>

// Local includes

#include "dwizardpage.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DatabasePage : public DWizardPage
{
    Q_OBJECT

public:

    explicit DatabasePage(QWizard* const dlg);
    ~DatabasePage() override;

    bool checkSettings();
    void saveSettings();

    void setDatabasePath(const QString& path);

    DbEngineParameters getDbEngineParameters() const;

private:

    bool checkLocalDatabase(QString& dbFolder);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DATABASE_PAGE_H
