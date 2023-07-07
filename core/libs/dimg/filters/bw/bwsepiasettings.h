/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-23
 * Description : black and white settings view.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BW_SEPIA_SETTINGS_H
#define DIGIKAM_BW_SEPIA_SETTINGS_H

// Local includes

#include <QWidget>

// Local includes

#include "digikam_export.h"
#include "bwsepiafilter.h"
#include "dimg.h"

class KConfigGroup;

namespace Digikam
{

class DIGIKAM_EXPORT BWSepiaSettings : public QWidget
{
    Q_OBJECT

public:

    explicit BWSepiaSettings(QWidget* const parent, DImg* const img);
    ~BWSepiaSettings() override;

    BWSepiaContainer defaultSettings() const;
    void resetToDefault();

    BWSepiaContainer settings()        const;
    void setSettings(const BWSepiaContainer& settings);

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

    void loadSettings();
    void saveAsSettings();

    void setScaleType(HistogramScale scale);

    void startPreviewFilters();

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotFilterSelected();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BW_SEPIA_SETTINGS_H
