/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-21
 * Description : Anti-vignetting batch tool.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "antivignetting.h"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "antivignettingfilter.h"

namespace DigikamBqmAntiVignettingPlugin
{

AntiVignetting::AntiVignetting(QObject* const parent)
    : BatchTool(QLatin1String("AntiVignetting"), EnhanceTool, parent),
      m_settingsView(nullptr)
{
}

AntiVignetting::~AntiVignetting()
{
}

BatchTool* AntiVignetting::clone(QObject* const parent) const
{
    return new AntiVignetting(parent);
}

void AntiVignetting::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new AntiVignettingSettings(m_settingsWidget);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings AntiVignetting::defaultSettings()
{
    BatchToolSettings prm;
    AntiVignettingContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("addvignetting"), (bool)defaultPrm.addvignetting);
    prm.insert(QLatin1String("density"),       (double)defaultPrm.density);
    prm.insert(QLatin1String("power"),         (double)defaultPrm.power);
    prm.insert(QLatin1String("innerradius"),   (double)defaultPrm.innerradius);
    prm.insert(QLatin1String("outerradius"),   (double)defaultPrm.outerradius);
    prm.insert(QLatin1String("xshift"),        (double)defaultPrm.xshift);
    prm.insert(QLatin1String("yshift"),        (double)defaultPrm.yshift);

    return prm;
}

void AntiVignetting::slotAssignSettings2Widget()
{
    AntiVignettingContainer prm;
    prm.addvignetting = settings()[QLatin1String("addvignetting")].toBool();
    prm.density       = settings()[QLatin1String("density")].toDouble();
    prm.power         = settings()[QLatin1String("power")].toDouble();
    prm.innerradius   = settings()[QLatin1String("innerradius")].toDouble();
    prm.outerradius   = settings()[QLatin1String("outerradius")].toDouble();
    prm.xshift        = settings()[QLatin1String("xshift")].toDouble();
    prm.yshift        = settings()[QLatin1String("yshift")].toDouble();
    m_settingsView->setSettings(prm);
}

void AntiVignetting::slotSettingsChanged()
{
    BatchToolSettings prm;
    AntiVignettingContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("addvignetting"), (bool)currentPrm.addvignetting);
    prm.insert(QLatin1String("density"),       (double)currentPrm.density);
    prm.insert(QLatin1String("power"),         (double)currentPrm.power);
    prm.insert(QLatin1String("innerradius"),   (double)currentPrm.innerradius);
    prm.insert(QLatin1String("outerradius"),   (double)currentPrm.outerradius);
    prm.insert(QLatin1String("xshift"),        (double)currentPrm.xshift);
    prm.insert(QLatin1String("yshift"),        (double)currentPrm.yshift);

    BatchTool::slotSettingsChanged(prm);
}

bool AntiVignetting::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    AntiVignettingContainer prm;
    prm.addvignetting = settings()[QLatin1String("addvignetting")].toBool();
    prm.density       = settings()[QLatin1String("density")].toDouble();
    prm.power         = settings()[QLatin1String("power")].toDouble();
    prm.innerradius   = settings()[QLatin1String("innerradius")].toDouble();
    prm.outerradius   = settings()[QLatin1String("outerradius")].toDouble();
    prm.xshift        = settings()[QLatin1String("xshift")].toDouble();
    prm.yshift        = settings()[QLatin1String("yshift")].toDouble();

    AntiVignettingFilter vig(&image(), nullptr, prm);
    applyFilter(&vig);

    return (savefromDImg());
}

} // namespace DigikamBqmAntiVignettingPlugin
