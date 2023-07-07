/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Wavelets Noise Reduction batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_NOISE_REDUCTION_H
#define DIGIKAM_BQM_NOISE_REDUCTION_H

// Local includes

#include "batchtool.h"
#include "nrsettings.h"

using namespace Digikam;

namespace DigikamBqmNoiseReductionPlugin
{

class NoiseReduction : public BatchTool
{
    Q_OBJECT

public:

    explicit NoiseReduction(QObject* const parent = nullptr);
    ~NoiseReduction()                                       override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    NRSettings* m_settingsView;
};

} // namespace DigikamBqmNoiseReductionPlugin

#endif // DIGIKAM_BQM_NOISE_REDUCTION_H
