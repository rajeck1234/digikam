/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 16/08/2016
 * Description : A Red-Eye tool for automatic detection and correction filter.
 *
 * SPDX-FileCopyrightText: 2016 by Omar Amin <Omar dot moh dot amin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_RED_EYE_CORRECTION_H
#define DIGIKAM_BQM_RED_EYE_CORRECTION_H

// Local includes

#include "batchtool.h"
#include "redeyecorrectionfilter.h"
#include "redeyecorrectionsettings.h"

using namespace Digikam;

namespace DigikamBqmRedEyeCorrectionPlugin
{

class RedEyeCorrection : public BatchTool
{
    Q_OBJECT

public:

    explicit RedEyeCorrection(QObject* const parent = nullptr);
    ~RedEyeCorrection()                                     override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

    void cancel()                                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    RedEyeCorrectionFilter*   m_redEyeCFilter;
    RedEyeCorrectionSettings* m_settingsView;
};

} // namespace DigikamBqmRedEyeCorrectionPlugin

#endif // DIGIKAM_BQM_RED_EYE_CORRECTION_H
