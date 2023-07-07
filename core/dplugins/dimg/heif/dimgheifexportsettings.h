/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-10-02
 * Description : HEIF image export settings widget.
 *
 * SPDX-FileCopyrightText: 2020-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIMG_HEIF_EXPORT_SETTINGS_H
#define DIGIKAM_DIMG_HEIF_EXPORT_SETTINGS_H

// Local includes

#include "dimgloadersettings.h"

namespace Digikam
{

class DImgHEIFExportSettings : public DImgLoaderSettings
{
    Q_OBJECT

public:

    explicit DImgHEIFExportSettings(QWidget* const parent = nullptr);
    ~DImgHEIFExportSettings()                   override;

    /**
     * This widget manage 2 parameters for the image encoding:
     * "quality"  as integer [1 - 100].
     * "lossless" as boolean.
     */
    void setSettings(const DImgLoaderPrms& set) override;
    DImgLoaderPrms settings() const             override;

private Q_SLOTS:

    void slotToggleHEIFLossLess(bool);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DIMG_HEIF_EXPORT_SETTINGS_H
