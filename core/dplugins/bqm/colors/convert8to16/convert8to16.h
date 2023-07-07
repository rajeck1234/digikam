/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-16
 * Description : 8 to 16 bits color depth converter batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_CONVERT_8_TO_16_H
#define DIGIKAM_BQM_CONVERT_8_TO_16_H

// Local includes

#include "batchtool.h"

using namespace Digikam;

namespace DigikamBqmConvert8To16Plugin
{

class Convert8to16 : public BatchTool
{
    Q_OBJECT

public:

    explicit Convert8to16(QObject* const parent = nullptr);
    ~Convert8to16()                                         override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override {};
    void slotSettingsChanged()                              override {};
};

} // namespace DigikamBqmConvert8To16Plugin

#endif // DIGIKAM_BQM_CONVERT_8_TO_16_H
