/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : auto colors correction batch tool.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_AUTO_CORRECTION_H
#define DIGIKAM_BQM_AUTO_CORRECTION_H

// Local includes

#include "batchtool.h"

class QComboBox;

using namespace Digikam;

namespace DigikamBqmAutoCorrectionPlugin
{

class AutoCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit AutoCorrection(QObject* const parent = nullptr);
    ~AutoCorrection()                                       override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    enum AutoCorrectionType
    {
        AutoLevelsCorrection = 0,
        NormalizeCorrection,
        EqualizeCorrection,
        StretchContrastCorrection,
        AutoExposureCorrection
    };

    QComboBox* m_comboBox;
};

} // namespace DigikamBqmAutoCorrectionPlugin

#endif // DIGIKAM_BQM_AUTO_CORRECTION_H
