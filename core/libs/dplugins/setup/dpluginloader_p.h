/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : manager to load external plugins at run-time: private container
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DPLUGIN_LOADER_P_H
#define DIGIKAM_DPLUGIN_LOADER_P_H

#include "dpluginloader.h"

// Qt includes

#include <QDir>
#include <QPluginLoader>

namespace Digikam
{

class Q_DECL_HIDDEN DPluginLoader::Private
{
public:

    explicit Private();
    ~Private();

    /**
     * Try to find plugin files from Qt5 plugins install dir:
     */
    QFileInfoList pluginEntriesList() const;

    /**
     * Stage to load plugins in memory
     */
    void loadPlugins();

    /**
     * Append object to the given plugins list.
     */
    bool appendPlugin(QObject* const obj,
                      QPluginLoader* const loader);

public:

    bool                  pluginsLoaded;
    QList<DPlugin*>       allPlugins;
    QStringList           blacklist;
    QStringList           whitelist;
    QStringList           DKBlacklist;      ///< Showfoto specific plugins to ignore in digiKam.
};

} // namespace Digikam

#endif // DIGIKAM_DPLUGIN_LOADER_P_H
