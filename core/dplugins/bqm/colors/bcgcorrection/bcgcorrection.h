/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Brightness/Contrast/Gamma batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_BCG_CORRECTION_H
#define DIGIKAM_BQM_BCG_CORRECTION_H

// Local includes

#include "batchtool.h"
#include "bcgsettings.h"

using namespace Digikam;

namespace DigikamBqmBCGCorrectionPlugin
{

class BCGCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit BCGCorrection(QObject* const parent = nullptr);
    ~BCGCorrection()                                        override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    BCGSettings* m_settingsView;
};

} // namespace DigikamBqmBCGCorrectionPlugin

#endif // DIGIKAM_BQM_BCG_CORRECTION_H
