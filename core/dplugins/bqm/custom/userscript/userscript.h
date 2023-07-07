/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-24
 * Description : user script batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2014      by Hubert Law <hhclaw dot eb at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_USER_SCRIPT_H
#define DIGIKAM_BQM_USER_SCRIPT_H

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmUserScriptPlugin
{

class UserScript : public BatchTool
{
    Q_OBJECT

public:

    explicit UserScript(QObject* const parent = nullptr);
    ~UserScript()                                           override;

    QString outputSuffix()                            const override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamBqmUserScriptPlugin

#endif // DIGIKAM_BQM_USER_SCRIPT_H
