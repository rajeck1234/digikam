/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a tool to reduce CCD noise.
 *
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "noisereductiontool.h"

// Qt includes

#include <QString>
#include <QIcon>
#include <QApplication>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "dimg.h"
#include "nrestimate.h"
#include "nrsettings.h"
#include "nrfilter.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageregionwidget.h"

namespace DigikamEditorNoiseReductionToolPlugin
{

class Q_DECL_HIDDEN NoiseReductionTool::Private
{
public:

    explicit Private()
      : configGroupName(QLatin1String("noisereduction Tool")),
        nrSettings     (nullptr),
        previewWidget  (nullptr),
        gboxSettings  (nullptr)
    {
    }

    const QString       configGroupName;

    NRSettings*         nrSettings;
    ImageRegionWidget*  previewWidget;
    EditorToolSettings* gboxSettings;
};

NoiseReductionTool::NoiseReductionTool(QObject* const parent)
    : EditorToolThreaded(parent),
      d                 (new Private)
{
    setObjectName(QLatin1String("noisereduction"));
    setToolName(i18n("Noise Reduction"));
    setToolIcon(QIcon::fromTheme(QLatin1String("noisereduction")));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings(nullptr);
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Cancel|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Try);

    d->nrSettings    = new NRSettings(d->gboxSettings->plainPage());
    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    connect(d->nrSettings, SIGNAL(signalEstimateNoise()),
            this, SLOT(slotEstimateNoise()));
}

NoiseReductionTool::~NoiseReductionTool()
{
    delete d;
}

void NoiseReductionTool::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    d->nrSettings->readSettings(group);
}

void NoiseReductionTool::writeSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->nrSettings->writeSettings(group);
    group.sync();
}

void NoiseReductionTool::slotResetSettings()
{
    d->nrSettings->resetToDefault();
}

void NoiseReductionTool::preparePreview()
{
    DImg image      = d->previewWidget->getOriginalRegionImage();
    NRContainer prm = d->nrSettings->settings();

    setFilter(new NRFilter(&image, this, prm));
}

void NoiseReductionTool::prepareFinal()
{
    NRContainer prm = d->nrSettings->settings();

    ImageIface iface;
    setFilter(new NRFilter(iface.original(), this, prm));
}

void NoiseReductionTool::setPreviewImage()
{
    d->previewWidget->setPreviewImage(filter()->getTargetImage());
}

void NoiseReductionTool::setFinalImage()
{
    ImageIface iface;
    iface.setOriginal(i18n("Noise Reduction"), filter()->filterAction(), filter()->getTargetImage());
}

void NoiseReductionTool::slotLoadSettings()
{
    d->nrSettings->loadSettings();
}

void NoiseReductionTool::slotSaveAsSettings()
{
    d->nrSettings->saveAsSettings();
}

void NoiseReductionTool::slotEstimateNoise()
{
    ImageIface iface;
    setAnalyser(new NREstimate(iface.original(), this));
}

void NoiseReductionTool::analyserCompleted()
{
    NREstimate* const tool = dynamic_cast<NREstimate*>(analyser());
    if (!tool) return;

    d->nrSettings->setSettings(tool->settings());
    qApp->restoreOverrideCursor();
    slotPreview();
}

} // namespace DigikamEditorNoiseReductionToolPlugin
