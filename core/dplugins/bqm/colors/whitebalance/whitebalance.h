/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-27
 * Description : White Balance batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_WHITE_BALANCE_H
#define DIGIKAM_BQM_WHITE_BALANCE_H

// Local includes

#include "batchtool.h"
#include "wbsettings.h"

using namespace Digikam;

namespace DigikamBqmWhiteBalancePlugin
{

class WhiteBalance : public BatchTool
{
    Q_OBJECT

public:

    explicit WhiteBalance(QObject* const parent = nullptr);
    ~WhiteBalance()                                         override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    WBSettings* m_settingsView;
};

} // namespace DigikamBqmWhiteBalancePlugin

#endif // DIGIKAM_BQM_WHITE_BALANCE_H
