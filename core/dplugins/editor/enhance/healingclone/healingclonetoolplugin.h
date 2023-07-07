/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : image editor plugin to reduce image artifacts
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_HEALINGCLONETOOL_PLUGIN_H
#define DIGIKAM_HEALINGCLONETOOL_PLUGIN_H

// Local includes

#include "dplugineditor.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.editor.HealingCloneTool"

using namespace Digikam;

namespace DigikamEditorHealingCloneToolPlugin
{

class HealingCloneToolPlugin : public DPluginEditor
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginEditor)

public:

    explicit HealingCloneToolPlugin(QObject* const parent = nullptr);
    ~HealingCloneToolPlugin()            override;

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

private Q_SLOTS:

    void slotHealingClone();
};

} // namespace DigikamEditorHealingCloneToolPlugin

#endif // DIGIKAM_HEALINGCLONETOOL_PLUGIN_H
