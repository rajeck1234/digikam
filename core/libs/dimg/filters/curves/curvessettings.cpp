/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-01
 * Description : Curves settings view.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "curvessettings.h"

// Qt includes

#include <QString>
#include <QButtonGroup>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QTextStream>
#include <QToolButton>
#include <QVBoxLayout>
#include <QStandardPaths>
#include <QApplication>
#include <QStyle>
#include <QUrl>
#include <QMessageBox>

// KDE includes

#include <kconfiggroup.h>
#include <klocalizedstring.h>

// Local includes

#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dnuminput.h"
#include "digikam_debug.h"
#include "dcombobox.h"
#include "colorgradientwidget.h"

namespace Digikam
{

class Q_DECL_HIDDEN CurvesSettings::Private
{

public:

    explicit Private()
      : histoSegments(0),
        curvesBox(nullptr)
    {
    }

    static const QString configCurveEntry;

    int                  histoSegments;
    CurvesBox*           curvesBox;
};

const QString CurvesSettings::Private::configCurveEntry(QLatin1String("AdjustCurves"));

// --------------------------------------------------------

CurvesSettings::CurvesSettings(QWidget* const parent, DImg* const img)
    : QWidget(parent),
      d(new Private)
{
    d->histoSegments        = img->sixteenBit() ? 65535 : 255;
    QGridLayout* const grid = new QGridLayout(this);

    // NOTE: add a method to be able to use curves widget without image data as simple curve editor.

    if (!img->isNull())
    {
        d->curvesBox = new CurvesBox(256, 192, *img);
        d->curvesBox->enableControlWidgets(true);
    }
    else
    {
        d->curvesBox = new CurvesBox(256, 192, DImg(1, 1, true, false, (uchar*)"\x00\x00\x00\x00\x00\x00\x00\x00"));
        d->curvesBox->enablePickers(false);
        d->curvesBox->enableResetButton(true);
        d->curvesBox->enableCurveTypes(true);
    }

    d->curvesBox->enableGradients(true);

    grid->addWidget(d->curvesBox, 0, 0, 1, 1);
    grid->setRowStretch(1, 10);
    grid->setContentsMargins(QMargins());
    grid->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));

    // -------------------------------------------------------------

    // NOTE: no need to Q_EMIT signalSettingsChanged() at CurveBox::signalChannelReset()
    // and CurveBox::signalCurveTypeChanged(), it's managed with CurveBox::signalCurvesChanged()

    connect(d->curvesBox, SIGNAL(signalCurvesChanged()),
            this, SIGNAL(signalSettingsChanged()));

    connect(d->curvesBox, SIGNAL(signalChannelReset(int)),
            this, SIGNAL(signalChannelReset(int)));

    connect(d->curvesBox, SIGNAL(signalPickerChanged(int)),
            this, SIGNAL(signalPickerChanged(int)));
}

CurvesSettings::~CurvesSettings()
{
    delete d;
}

void CurvesSettings::setCurrentChannel(ChannelType channel)
{
    d->curvesBox->setChannel(channel);
}

void CurvesSettings::setScale(HistogramScale type)
{
    d->curvesBox->setScale(type);
}

void CurvesSettings::slotSpotColorChanged(const DColor& color)
{
    int maxColor = qMax(qMax(color.red(), color.green()), color.blue());

    switch (d->curvesBox->picker())
    {
        case CurvesBox::BlackTonal:
        {
            // Black tonal curves point.

            int segment = 42 * d->histoSegments / 256;
            d->curvesBox->curves()->setCurvePoint(LuminosityChannel,  1, QPoint(maxColor,      segment));
            d->curvesBox->curves()->setCurvePoint(RedChannel,         1, QPoint(color.red(),   segment));
            d->curvesBox->curves()->setCurvePoint(GreenChannel,       1, QPoint(color.green(), segment));
            d->curvesBox->curves()->setCurvePoint(BlueChannel,        1, QPoint(color.blue(),  segment));
            d->curvesBox->resetPickers();
            break;
        }

        case CurvesBox::GrayTonal:
        {
            // Gray tonal curves point.

            int segment = 128 * d->histoSegments / 256;
            d->curvesBox->curves()->setCurvePoint(LuminosityChannel,  8, QPoint(maxColor,      segment));
            d->curvesBox->curves()->setCurvePoint(RedChannel,         8, QPoint(color.red(),   segment));
            d->curvesBox->curves()->setCurvePoint(GreenChannel,       8, QPoint(color.green(), segment));
            d->curvesBox->curves()->setCurvePoint(BlueChannel,        8, QPoint(color.blue(),  segment));
            d->curvesBox->resetPickers();
            break;
        }

        case CurvesBox::WhiteTonal:
        {
            // White tonal curves point.

            int segment = 213 * d->histoSegments / 256;
            d->curvesBox->curves()->setCurvePoint(LuminosityChannel, 15, QPoint(maxColor,      segment));
            d->curvesBox->curves()->setCurvePoint(RedChannel,        15, QPoint(color.red(),   segment));
            d->curvesBox->curves()->setCurvePoint(GreenChannel,      15, QPoint(color.green(), segment));
            d->curvesBox->curves()->setCurvePoint(BlueChannel,       15, QPoint(color.blue(),  segment));
            d->curvesBox->resetPickers();
            break;
        }

        default:
        {
            d->curvesBox->setCurveGuide(color);
            return;
        }
    }

    // Calculate Red, green, blue curves.

    for (int i = LuminosityChannel ; i <= BlueChannel ; ++i)
    {
        d->curvesBox->curves()->curvesCalculateCurve(i);
    }

    d->curvesBox->repaint();
    d->curvesBox->resetPickers();

    Q_EMIT signalSpotColorChanged();
}

CurvesContainer CurvesSettings::settings() const
{
    d->curvesBox->curves()->curvesCalculateAllCurves();
    return d->curvesBox->curves()->getContainer();
}

void CurvesSettings::setSettings(const CurvesContainer& settings)
{
    blockSignals(true);
    d->curvesBox->curves()->setContainer(settings);
    blockSignals(false);
}

void CurvesSettings::resetToDefault()
{
    blockSignals(true);
    d->curvesBox->resetChannels();
    d->curvesBox->resetPickers();
    blockSignals(false);
}

CurvesContainer CurvesSettings::defaultSettings() const
{
    CurvesContainer prm;
    return prm;
}

void CurvesSettings::readSettings(KConfigGroup& group)
{
    d->curvesBox->reset();
    d->curvesBox->readCurveSettings(group, d->configCurveEntry);
    d->curvesBox->update();
}

void CurvesSettings::writeSettings(KConfigGroup& group)
{
    d->curvesBox->writeCurveSettings(group, d->configCurveEntry);
}

void CurvesSettings::loadSettings()
{
    QUrl loadCurvesFile;

    loadCurvesFile = DFileDialog::getOpenFileUrl(qApp->activeWindow(), i18nc("@title:window", "Select Gimp Curves File to Load"),
                                                 QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                 QLatin1String("*"));

    if (loadCurvesFile.isEmpty())
    {
        return;
    }

    if (d->curvesBox->curves()->loadCurvesFromGimpCurvesFile(loadCurvesFile) == false)
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot load from the Gimp curves text file."));
        return;
    }
}

void CurvesSettings::saveAsSettings()
{
    QUrl saveCurvesFile;

    saveCurvesFile = DFileDialog::getSaveFileUrl(qApp->activeWindow(), i18nc("@title:window", "Gimp Curves File to Save"),
                                                 QUrl::fromLocalFile(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)),
                                                 QLatin1String("*"));

    if (saveCurvesFile.isEmpty())
    {
        return;
    }

    if (d->curvesBox->curves()->saveCurvesToGimpCurvesFile(saveCurvesFile) == false)
    {
        QMessageBox::critical(qApp->activeWindow(), qApp->applicationName(),
                              i18n("Cannot save to the Gimp curves text file."));
        return;
    }
}

int CurvesSettings::curvesLeftOffset() const
{
    return d->curvesBox->curvesLeftOffset();
}

} // namespace Digikam
