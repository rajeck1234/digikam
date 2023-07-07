/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to edit items metadata.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_METADATA_EDIT_PLUGIN_H
#define DIGIKAM_METADATA_EDIT_PLUGIN_H

// Local includes

#include "dplugingeneric.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.MetadataEdit"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class MetadataEditPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit MetadataEditPlugin(QObject* const parent = nullptr);
    ~MetadataEditPlugin()                override;

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;

    void setup(QObject* const)           override;

private Q_SLOTS:

    void slotEditMetadata();
};

} // namespace DigikamGenericMetadataEditPlugin

#endif // DIGIKAM_METADATA_EDIT_PLUGIN_H
