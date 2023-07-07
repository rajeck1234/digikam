/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-30
 * Description : a plugin to create panorama.
 *
 * SPDX-FileCopyrightText: 2018-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "panoramaplugin.h"

// Qt includes

#include <QPointer>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "panomanager.h"

namespace DigikamGenericPanoramaPlugin
{

PanoramaPlugin::PanoramaPlugin(QObject* const parent)
    : DPluginGeneric(parent)
{
}

PanoramaPlugin::~PanoramaPlugin()
{
}

void PanoramaPlugin::cleanUp()
{
    if (PanoManager::isCreated())
    {
        delete PanoManager::internalPtr;
    }
}

QString PanoramaPlugin::name() const
{
    return i18n("Create Panorama");
}

QString PanoramaPlugin::iid() const
{
    return QLatin1String(DPLUGIN_IID);
}

QIcon PanoramaPlugin::icon() const
{
    return QIcon::fromTheme(QLatin1String("panorama"));
}

QString PanoramaPlugin::description() const
{
    return i18n("A tool to create panorama");
}

QString PanoramaPlugin::details() const
{
    return i18n("<p>This tool allows users to assemble images together to create large panorama.</p>"
                "<p>To create panorama image, you need to use images taken from same point of "
                "view with a tripod and exposed with same settings.</p>"
                "<p>The tool is able to assemble shot taken horizontally, vertically, or as a "
                "matrix. Take a care that target image will become huge and require a lots of "
                "memory to be processed.</p>");
}

QString PanoramaPlugin::handbookSection() const
{
    return QLatin1String("post_processing");
}

QString PanoramaPlugin::handbookChapter() const
{
    return QLatin1String("panorama_tool");
}

QList<DPluginAuthor> PanoramaPlugin::authors() const
{
    return QList<DPluginAuthor>()
            << DPluginAuthor(QString::fromUtf8("Benjamin Girault"),
                             QString::fromUtf8("benjamin dot girault at gmail dot com"),
                             QString::fromUtf8("(C) 2011-2016"))
            << DPluginAuthor(QString::fromUtf8("Gilles Caulier"),
                             QString::fromUtf8("caulier dot gilles at gmail dot com"),
                             QString::fromUtf8("(C) 2009-2020"),
                             i18n("Author and Maintainer"))
            ;
}

void PanoramaPlugin::setup(QObject* const parent)
{
    DPluginAction* const ac = new DPluginAction(parent);
    ac->setIcon(icon());
    ac->setText(i18nc("@action", "Create panorama..."));
    ac->setObjectName(QLatin1String("panorama"));
    ac->setActionCategory(DPluginAction::GenericTool);

    connect(ac, SIGNAL(triggered(bool)),
            this, SLOT(slotPanorama()));

    addAction(ac);
}

void PanoramaPlugin::slotPanorama()
{
    DInfoInterface* const iface = infoIface(sender());
    bool created                = PanoManager::isCreated();

    PanoManager::instance()->checkBinaries();
    PanoManager::instance()->setItemsList(iface->currentSelectedItems());
    PanoManager::instance()->setPlugin(this);

    if (!created)
    {
        connect(PanoManager::instance(), SIGNAL(updateHostApp(QUrl)),
                iface, SLOT(slotMetadataChangedForUrl(QUrl)));
    }

    PanoManager::instance()->run();
}

} // namespace DigikamGenericPanoramaPlugin
