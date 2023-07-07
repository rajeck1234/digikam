/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-17
 * Description : batch tool to add border.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_BORDER_H
#define DIGIKAM_BQM_BORDER_H

// Local includes

#include "batchtool.h"
#include "bordersettings.h"

using namespace Digikam;

namespace DigikamBqmBorderPlugin
{

class Border : public BatchTool
{
    Q_OBJECT

public:

    explicit Border(QObject* const parent = nullptr);
    ~Border()                                               override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotSettingsChanged()                              override;
    void slotAssignSettings2Widget()                        override;

private:

    bool toolOperations()                                   override;

private:

    BorderSettings* m_settingsView;
};

} // namespace DigikamBqmBorderPlugin

#endif // DIGIKAM_BQM_BORDER_H
