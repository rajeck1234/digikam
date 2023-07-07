/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : BCG settings view.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "bcgsettings.h"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QCheckBox>
#include <QUrl>
#include <QApplication>
#include <QStyle>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dnuminput.h"
#include "digikam_debug.h"
#include "dexpanderbox.h"

namespace Digikam
{

class Q_DECL_HIDDEN BCGSettings::Private
{
public:

    explicit Private()
      : bInput(nullptr),
        cInput(nullptr),
        gInput(nullptr)
    {
    }

    static const QString configBrightnessAdjustmentEntry;
    static const QString configContrastAdjustmentEntry;
    static const QString configGammaAdjustmentEntry;

    DIntNumInput*        bInput;
    DIntNumInput*        cInput;

    DDoubleNumInput*     gInput;
};

const QString BCGSettings::Private::configBrightnessAdjustmentEntry(QLatin1String("BrightnessAdjustment"));
const QString BCGSettings::Private::configContrastAdjustmentEntry(QLatin1String("ContrastAdjustment"));
const QString BCGSettings::Private::configGammaAdjustmentEntry(QLatin1String("GammaAdjustment"));

// --------------------------------------------------------

BCGSettings::BCGSettings(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    const int spacing       = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    QGridLayout* const grid = new QGridLayout(parent);

    QLabel* const label2 = new QLabel(i18n("Brightness:"));
    d->bInput            = new DIntNumInput();
    d->bInput->setRange(-100, 100, 1);
    d->bInput->setDefaultValue(0);
    d->bInput->setWhatsThis(i18n("Set here the brightness adjustment of the image."));

    QLabel* const label3 = new QLabel(i18n("Contrast:"));
    d->cInput            = new DIntNumInput();
    d->cInput->setRange(-100, 100, 1);
    d->cInput->setDefaultValue(0);
    d->cInput->setWhatsThis(i18n("Set here the contrast adjustment of the image."));

    QLabel* const label4 = new QLabel(i18n("Gamma:"));
    d->gInput            = new DDoubleNumInput();
    d->gInput->setDecimals(2);
    d->gInput->setRange(0.1, 3.0, 0.01);
    d->gInput->setDefaultValue(1.0);
    d->gInput->setWhatsThis(i18n("Set here the gamma adjustment of the image."));

    // -------------------------------------------------------------

    grid->addWidget(label2,    0, 0, 1, 5);
    grid->addWidget(d->bInput, 1, 0, 1, 5);
    grid->addWidget(label3,    2, 0, 1, 5);
    grid->addWidget(d->cInput, 3, 0, 1, 5);
    grid->addWidget(label4,    4, 0, 1, 5);
    grid->addWidget(d->gInput, 5, 0, 1, 5);
    grid->setRowStretch(6, 10);
    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // -------------------------------------------------------------

    connect(d->bInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->cInput, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->gInput, SIGNAL(valueChanged(double)),
            this, SIGNAL(signalSettingsChanged()));
}

BCGSettings::~BCGSettings()
{
    delete d;
}

BCGContainer BCGSettings::settings() const
{
    BCGContainer prm;

    prm.brightness = (double)d->bInput->value()  / 250.0;
    prm.contrast   = (double)(d->cInput->value() / 100.0) + 1.00;
    prm.gamma      = d->gInput->value();

    return prm;
}

void BCGSettings::setSettings(const BCGContainer& settings)
{
    blockSignals(true);
    d->bInput->setValue((int)(settings.brightness * 250.0));
    d->cInput->setValue((int)((settings.contrast - 1.0) * 100.0));
    d->gInput->setValue(settings.gamma);
    blockSignals(false);
}

void BCGSettings::resetToDefault()
{
    blockSignals(true);
    d->bInput->slotReset();
    d->cInput->slotReset();
    d->gInput->slotReset();
    blockSignals(false);
}

BCGContainer BCGSettings::defaultSettings() const
{
    BCGContainer prm;

    prm.brightness = (double)(d->bInput->defaultValue() / 250.0);
    prm.contrast   = (double)(d->cInput->defaultValue() / 100.0) + 1.00;
    prm.gamma      = d->gInput->defaultValue();

    return prm;
}

void BCGSettings::readSettings(const KConfigGroup& group)
{
    BCGContainer prm;
    BCGContainer defaultPrm = defaultSettings();
    prm.brightness          = group.readEntry(d->configBrightnessAdjustmentEntry, defaultPrm.brightness);
    prm.contrast            = group.readEntry(d->configContrastAdjustmentEntry,   defaultPrm.contrast);
    prm.gamma               = group.readEntry(d->configGammaAdjustmentEntry,      defaultPrm.gamma);

    setSettings(prm);
}

void BCGSettings::writeSettings(KConfigGroup& group)
{
    BCGContainer prm = settings();

    group.writeEntry(d->configBrightnessAdjustmentEntry, prm.brightness);
    group.writeEntry(d->configContrastAdjustmentEntry,   prm.contrast);
    group.writeEntry(d->configGammaAdjustmentEntry,      prm.gamma);
}

} // namespace Digikam
