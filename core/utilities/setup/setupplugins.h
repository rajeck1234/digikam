/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Setup view panel for dplugins.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SETUP_PLUGINS_H
#define DIGIKAM_SETUP_PLUGINS_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupPlugins : public QScrollArea
{
    Q_OBJECT

public:

    enum PluginTab
    {
        Generic = 0,
        Editor,
        Bqm,
        Loaders
    };

public:

    explicit SetupPlugins(QWidget* const parent = nullptr);
    ~SetupPlugins() override;

    void setActiveTab(PluginTab tab);
    PluginTab activeTab() const;

    void applySettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SETUP_PLUGINS_H
