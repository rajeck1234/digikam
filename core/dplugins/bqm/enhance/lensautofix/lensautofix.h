/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-09-18
 * Description : lens auto-fix batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_LENS_AUTO_FIX_H
#define DIGIKAM_BQM_LENS_AUTO_FIX_H

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmLensAutoFixPlugin
{

class LensAutoFix : public BatchTool
{
    Q_OBJECT

public:

    explicit LensAutoFix(QObject* const parent = nullptr);
    ~LensAutoFix()                                          override;

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

} // namespace DigikamBqmLensAutoFixPlugin

#endif // DIGIKAM_BQM_LENS_AUTO_FIX_H
