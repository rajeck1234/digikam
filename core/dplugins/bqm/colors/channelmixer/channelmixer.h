/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-19
 * Description : Channel Mixer batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_CHANNEL_MIXER_H
#define DIGIKAM_BQM_CHANNEL_MIXER_H

// Local includes

#include "batchtool.h"
#include "mixersettings.h"

using namespace Digikam;

namespace DigikamBqmChannelMixerPlugin
{

class ChannelMixer : public BatchTool
{
    Q_OBJECT

public:

    explicit ChannelMixer(QObject* const parent = nullptr);
    ~ChannelMixer()                                         override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private:

    bool toolOperations()                                   override;

private Q_SLOTS:

    void slotAssignSettings2Widget()                        override;
    void slotSettingsChanged()                              override;

private:

    MixerSettings* m_settingsView;
};

} // namespace DigikamBqmChannelMixerPlugin

#endif // DIGIKAM_BQM_CHANNEL_MIXER_H
