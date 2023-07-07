/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 16 to 8 bits color depth converter batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_CONVERT_16TO8_H
#define DIGIKAM_BQM_CONVERT_16TO8_H

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmConvert16To8Plugin
{

class Convert16to8 : public BatchTool
{
    Q_OBJECT

public:

    explicit Convert16to8(QObject* const parent = nullptr);
    ~Convert16to8()                                         override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override {};
    void slotSettingsChanged()                              override {};
};

} // namespace DigikamBqmConvert16To8Plugin

#endif // DIGIKAM_BQM_CONVERT_16TO8_H
