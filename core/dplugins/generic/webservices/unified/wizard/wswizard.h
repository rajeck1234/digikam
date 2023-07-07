/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018      by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_WS_WIZARD_H
#define DIGIKAM_WS_WIZARD_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QSettings>

// Local includes

#include "dwizarddlg.h"
#include "dinfointerface.h"
#include "o0settingsstore.h"
#include "wssettings.h"
#include "wsauthentication.h"

using namespace Digikam;

namespace DigikamGenericUnifiedPlugin
{

class WSWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit WSWizard(DInfoInterface* const iface, QWidget* const parent);
    ~WSWizard();

    bool validateCurrentPage() override;
    int  nextId() const        override;

    DInfoInterface*     iface()              const;
    WSSettings*         settings()           const;

    /*
     * Instance of WSAuthentication (which wraps instance of WSTalker) and correspondent QSettings
     * are initialized only once in WSWizard.
     *
     * These 2 methods below are getters, used in other pages of wizard so as to facilitate
     * access to WSAuthentication instance and its settings.
     */
    WSAuthentication*   wsAuth()             const;
    QSettings*          oauthSettings()      const;
    O0SettingsStore*    oauthSettingsStore() const;

    void setItemsList(const QList<QUrl>& urls);

public Q_SLOTS:

    void slotBusy(bool val);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericUnifiedPlugin

#endif // DIGIKAM_WS_WIZARD_H
