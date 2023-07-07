/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2022-04-02
 * Description : a BQM plugin to convert to WEBP.
 *
 * SPDX-FileCopyrightText: 2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CONVERT_TO_WEBP_PLUGIN_H
#define DIGIKAM_CONVERT_TO_WEBP_PLUGIN_H

// Local includes

#include "dpluginbqm.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.bqm.ConvertToWebp"

using namespace Digikam;

namespace DigikamBqmConvertToWebpPlugin
{

class ConvertToWebpPlugin : public DPluginBqm
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginBqm)

public:

    explicit ConvertToWebpPlugin(QObject* const parent = nullptr);
    ~ConvertToWebpPlugin()               override;

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;
    QString handbookReference()    const override;

    void setup(QObject* const)           override;
};

} // namespace DigikamBqmConvertToWebpPlugin

#endif // DIGIKAM_CONVERT_TO_WEBP_PLUGIN_H
