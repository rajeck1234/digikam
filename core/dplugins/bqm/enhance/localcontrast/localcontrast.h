/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : LDR tonemapper batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_LOCAL_CONTRAST_H
#define DIGIKAM_BQM_LOCAL_CONTRAST_H

// Local includes

#include "batchtool.h"
#include "localcontrastsettings.h"

using namespace Digikam;

namespace DigikamBqmLocalContrastPlugin
{

class LocalContrast : public BatchTool
{
    Q_OBJECT

public:

    explicit LocalContrast(QObject* const parent = nullptr);
    ~LocalContrast()                                        override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    LocalContrastSettings* m_settingsView;
};

} // namespace DigikamBqmLocalContrastPlugin

#endif // DIGIKAM_BQM_LOCAL_CONTRAST_H
