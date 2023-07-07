/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2020-08-13
 * Description : batch tool to add texture.
 *
 * SPDX-FileCopyrightText: 2020-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_TEXTURE_H
#define DIGIKAM_BQM_TEXTURE_H

// Local includes

#include "batchtool.h"
#include "texturesettings.h"

using namespace Digikam;

namespace DigikamBqmTexturePlugin
{

class Texture : public BatchTool
{
    Q_OBJECT

public:

    explicit Texture(QObject* const parent = nullptr);
    ~Texture()                                              override;

    BatchToolSettings defaultSettings()                     override;

    BatchTool* clone(QObject* const parent = nullptr) const override;

    void registerSettingsWidget()                           override;

private Q_SLOTS:

    void slotSettingsChanged()                              override;
    void slotAssignSettings2Widget()                        override;

private:

    bool toolOperations()                                   override;

private:

    TextureSettings* m_settingsView;
};

} // namespace DigikamBqmTexturePlugin

#endif // DIGIKAM_BQM_TEXTURE_H
