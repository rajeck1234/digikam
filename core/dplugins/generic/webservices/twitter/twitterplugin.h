/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to export to Twitter web-service.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TWITTER_PLUGIN_H
#define DIGIKAM_TWITTER_PLUGIN_H

// Local includes

#include "dplugingeneric.h"
#include "dplugindialog.h"

#define DPLUGIN_IID "org.kde.digikam.plugin.generic.Twitter"

using namespace Digikam;

namespace DigikamGenericTwitterPlugin
{

class TwitterPlugin : public DPluginGeneric
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DPLUGIN_IID)
    Q_INTERFACES(Digikam::DPluginGeneric)

public:

    explicit TwitterPlugin(QObject* const parent = nullptr);
    ~TwitterPlugin()                     override;

    QString name()                 const override;
    QString iid()                  const override;
    QIcon   icon()                 const override;
    QString details()              const override;
    QString description()          const override;
    QList<DPluginAuthor> authors() const override;
    QString handbookSection()      const override;
    QString handbookChapter()      const override;

    void setup(QObject* const)           override;
    void cleanUp()                       override;

private Q_SLOTS:

    void slotTwitter();

private:

    QPointer<DPluginDialog> m_toolDlg;
};

} // namespace DigikamGenericTwitterPlugin

#endif // DIGIKAM_TWITTER_PLUGIN_H
