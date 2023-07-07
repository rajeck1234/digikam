/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * date        : 2006-09-13
 * Description : Raw Decoder settings widgets
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2007-2008 by Guillaume Castagnino <casta at xwing dot info>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DRAW_DECODER_WIDGET_H
#define DIGIKAM_DRAW_DECODER_WIDGET_H

// Qt includes

#include <QString>

// Local includes

#include "drawdecodersettings.h"
#include "dlayoutbox.h"
#include "dexpanderbox.h"
#include "dfileselector.h"
#include "digikam_export.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT DRawDecoderWidget : public DExpanderBox
{
    Q_OBJECT

public:

    enum AdvancedSettingsOptions
    {
        SIXTEENBITS      = 0x00000001,
        COLORSPACE       = 0x00000002,
        POSTPROCESSING   = 0x00000004,
        BLACKWHITEPOINTS = 0x00000008
    };

    enum SettingsTabs
    {
        DEMOSAICING = 0,
        WHITEBALANCE,
        CORRECTIONS,
        COLORMANAGEMENT
    };

public:

    /**
     * @param advSettings the default value is COLORSPACE
     */
    explicit DRawDecoderWidget(QWidget* const parent, int advSettings = COLORSPACE);
    ~DRawDecoderWidget()                        override;

    DFileSelector* inputProfileUrlEdit()  const;
    DFileSelector* outputProfileUrlEdit() const;

    void setup(int advSettings);

    void setEnabledBrightnessSettings(bool b);
    bool brightnessSettingsIsEnabled()    const;

    void updateMinimumWidth();

    void resetToDefault();

    void setSettings(const DRawDecoderSettings& settings);
    DRawDecoderSettings settings()        const;

    void readSettings(KConfigGroup& group)      override;
    void writeSettings(KConfigGroup& group)     override;

    static void readSettings(DRawDecoderSettings& setting, KConfigGroup& group);
    static void writeSettings(const DRawDecoderSettings& setting, KConfigGroup& group);

Q_SIGNALS:

    void signalSixteenBitsImageToggled(bool);
    void signalSettingsChanged();

private Q_SLOTS:

    void slotWhiteBalanceToggled(int);
    void slotsixteenBitsImageToggled(bool);
    void slotUnclipColorActivated(int);
    void slotNoiseReductionChanged(int);
    void slotExposureCorrectionToggled(bool);
    void slotInputColorSpaceChanged(int);
    void slotOutputColorSpaceChanged(int);
    void slotRAWQualityChanged(int);
    void slotExpoCorrectionShiftChanged(double);

private:

    class Private;
    Private* const d;
};

} // NameSpace Digikam

#endif // DIGIKAM_DRAW_DECODER_WIDGET_H
