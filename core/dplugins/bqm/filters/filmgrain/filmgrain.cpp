/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : a batch tool to add film grain to images.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "filmgrain.h"

// Qt includes

#include <QLabel>
#include <QGridLayout>
#include <QWidget>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dimg.h"
#include "filmgrainfilter.h"

namespace DigikamBqmFilmGrainPlugin
{

FilmGrain::FilmGrain(QObject* const parent)
    : BatchTool(QLatin1String("FilmGrain"), FiltersTool, parent),
      m_settingsView(nullptr)
{
}

FilmGrain::~FilmGrain()
{
}

BatchTool* FilmGrain::clone(QObject* const parent) const
{
    return new FilmGrain(parent);
}

void FilmGrain::registerSettingsWidget()
{
    m_settingsWidget = new QWidget;
    m_settingsView   = new FilmGrainSettings(m_settingsWidget);
    m_settingsView->resetToDefault();

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    BatchTool::registerSettingsWidget();
}

BatchToolSettings FilmGrain::defaultSettings()
{
    BatchToolSettings prm;
    FilmGrainContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert(QLatin1String("grainSize"),               (int)defaultPrm.grainSize);
    prm.insert(QLatin1String("photoDistribution"),       (bool)defaultPrm.photoDistribution);
    prm.insert(QLatin1String("addLuminanceNoise"),       (bool)defaultPrm.addLuminanceNoise);
    prm.insert(QLatin1String("lumaIntensity"),           (int)defaultPrm.lumaIntensity);
    prm.insert(QLatin1String("lumaShadows"),             (int)defaultPrm.lumaShadows);
    prm.insert(QLatin1String("lumaMidtones"),            (int)defaultPrm.lumaMidtones);
    prm.insert(QLatin1String("lumaHighlights"),          (int)defaultPrm.lumaHighlights);
    prm.insert(QLatin1String("addChrominanceBlueNoise"), (bool)defaultPrm.addChrominanceBlueNoise);
    prm.insert(QLatin1String("chromaBlueIntensity"),     (int)defaultPrm.chromaBlueIntensity);
    prm.insert(QLatin1String("chromaBlueShadows"),       (int)defaultPrm.chromaBlueShadows);
    prm.insert(QLatin1String("chromaBlueMidtones"),      (int)defaultPrm.chromaBlueMidtones);
    prm.insert(QLatin1String("chromaBlueHighlights"),    (int)defaultPrm.chromaBlueHighlights);
    prm.insert(QLatin1String("addChrominanceRedNoise"),  (bool)defaultPrm.addChrominanceRedNoise);
    prm.insert(QLatin1String("chromaRedIntensity"),      (int)defaultPrm.chromaRedIntensity);
    prm.insert(QLatin1String("chromaRedShadows"),        (int)defaultPrm.chromaRedShadows);
    prm.insert(QLatin1String("chromaRedMidtones"),       (int)defaultPrm.chromaRedMidtones);
    prm.insert(QLatin1String("chromaRedHighlights"),     (int)defaultPrm.chromaRedHighlights);

    return prm;
}

void FilmGrain::slotAssignSettings2Widget()
{
    FilmGrainContainer prm;
    prm.grainSize               = settings()[QLatin1String("grainSize")].toInt();
    prm.photoDistribution       = settings()[QLatin1String("photoDistribution")].toBool();
    prm.addLuminanceNoise       = settings()[QLatin1String("addLuminanceNoise")].toBool();
    prm.lumaIntensity           = settings()[QLatin1String("lumaIntensity")].toInt();
    prm.lumaShadows             = settings()[QLatin1String("lumaShadows")].toInt();
    prm.lumaMidtones            = settings()[QLatin1String("lumaMidtones")].toInt();
    prm.lumaHighlights          = settings()[QLatin1String("lumaHighlights")].toInt();
    prm.addChrominanceBlueNoise = settings()[QLatin1String("addChrominanceBlueNoise")].toBool();
    prm.chromaBlueIntensity     = settings()[QLatin1String("chromaBlueIntensity")].toInt();
    prm.chromaBlueShadows       = settings()[QLatin1String("chromaBlueShadows")].toInt();
    prm.chromaBlueMidtones      = settings()[QLatin1String("chromaBlueMidtones")].toInt();
    prm.chromaBlueHighlights    = settings()[QLatin1String("chromaBlueHighlights")].toInt();
    prm.addChrominanceRedNoise  = settings()[QLatin1String("addChrominanceRedNoise")].toBool();
    prm.chromaRedIntensity      = settings()[QLatin1String("chromaRedIntensity")].toInt();
    prm.chromaRedShadows        = settings()[QLatin1String("chromaRedShadows")].toInt();
    prm.chromaRedMidtones       = settings()[QLatin1String("chromaRedMidtones")].toInt();
    prm.chromaRedHighlights     = settings()[QLatin1String("chromaRedHighlights")].toInt();
    m_settingsView->setSettings(prm);
}

void FilmGrain::slotSettingsChanged()
{
    BatchToolSettings prm;
    FilmGrainContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("grainSize"),               (int)currentPrm.grainSize);
    prm.insert(QLatin1String("photoDistribution"),       (bool)currentPrm.photoDistribution);
    prm.insert(QLatin1String("addLuminanceNoise"),       (bool)currentPrm.addLuminanceNoise);
    prm.insert(QLatin1String("lumaIntensity"),           (int)currentPrm.lumaIntensity);
    prm.insert(QLatin1String("lumaShadows"),             (int)currentPrm.lumaShadows);
    prm.insert(QLatin1String("lumaMidtones"),            (int)currentPrm.lumaMidtones);
    prm.insert(QLatin1String("lumaHighlights"),          (int)currentPrm.lumaHighlights);
    prm.insert(QLatin1String("addChrominanceBlueNoise"), (bool)currentPrm.addChrominanceBlueNoise);
    prm.insert(QLatin1String("chromaBlueIntensity"),     (int)currentPrm.chromaBlueIntensity);
    prm.insert(QLatin1String("chromaBlueShadows"),       (int)currentPrm.chromaBlueShadows);
    prm.insert(QLatin1String("chromaBlueMidtones"),      (int)currentPrm.chromaBlueMidtones);
    prm.insert(QLatin1String("chromaBlueHighlights"),    (int)currentPrm.chromaBlueHighlights);
    prm.insert(QLatin1String("addChrominanceRedNoise"),  (bool)currentPrm.addChrominanceRedNoise);
    prm.insert(QLatin1String("chromaRedIntensity"),      (int)currentPrm.chromaRedIntensity);
    prm.insert(QLatin1String("chromaRedShadows"),        (int)currentPrm.chromaRedShadows);
    prm.insert(QLatin1String("chromaRedMidtones"),       (int)currentPrm.chromaRedMidtones);
    prm.insert(QLatin1String("chromaRedHighlights"),     (int)currentPrm.chromaRedHighlights);

    BatchTool::slotSettingsChanged(prm);
}

bool FilmGrain::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    FilmGrainContainer prm;
    prm.grainSize               = settings()[QLatin1String("grainSize")].toInt();
    prm.photoDistribution       = settings()[QLatin1String("photoDistribution")].toBool();
    prm.addLuminanceNoise       = settings()[QLatin1String("addLuminanceNoise")].toBool();
    prm.lumaIntensity           = settings()[QLatin1String("lumaIntensity")].toInt();
    prm.lumaShadows             = settings()[QLatin1String("lumaShadows")].toInt();
    prm.lumaMidtones            = settings()[QLatin1String("lumaMidtones")].toInt();
    prm.lumaHighlights          = settings()[QLatin1String("lumaHighlights")].toInt();
    prm.addChrominanceBlueNoise = settings()[QLatin1String("addChrominanceBlueNoise")].toBool();
    prm.chromaBlueIntensity     = settings()[QLatin1String("chromaBlueIntensity")].toInt();
    prm.chromaBlueShadows       = settings()[QLatin1String("chromaBlueShadows")].toInt();
    prm.chromaBlueMidtones      = settings()[QLatin1String("chromaBlueMidtones")].toInt();
    prm.chromaBlueHighlights    = settings()[QLatin1String("chromaBlueHighlights")].toInt();
    prm.addChrominanceRedNoise  = settings()[QLatin1String("addChrominanceRedNoise")].toBool();
    prm.chromaRedIntensity      = settings()[QLatin1String("chromaRedIntensity")].toInt();
    prm.chromaRedShadows        = settings()[QLatin1String("chromaRedShadows")].toInt();
    prm.chromaRedMidtones       = settings()[QLatin1String("chromaRedMidtones")].toInt();
    prm.chromaRedHighlights     = settings()[QLatin1String("chromaRedHighlights")].toInt();

    FilmGrainFilter fg(&image(), nullptr, prm);
    applyFilter(&fg);

    return savefromDImg();
}

} // namespace DigikamBqmFilmGrainPlugin
