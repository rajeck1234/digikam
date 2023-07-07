/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-02
 * Description : Curves Adjust batch tool.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "curvesadjust.h"

// Qt includes

#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "digikam_debug.h"
#include "dimg.h"
#include "curvesfilter.h"

namespace DigikamBqmCurvesAdjustPlugin
{

CurvesAdjust::CurvesAdjust(QObject* const parent)
    : BatchTool     (QLatin1String("CurvesAdjust"), ColorTool, parent),
      m_channelCB   (nullptr),
      m_settingsView(nullptr)
{
    setToolTitle(i18nc("@title", "Curves Adjust"));
    setToolDescription(i18nc("@info", "Perform curves adjustments."));
    setToolIconName(QLatin1String("adjustcurves"));
}

CurvesAdjust::~CurvesAdjust()
{
}

BatchTool* CurvesAdjust::clone(QObject* const parent) const
{
    return new CurvesAdjust(parent);
}

void CurvesAdjust::registerSettingsWidget()
{
    DVBox* const vbox          = new DVBox;
    DHBox* const hbox          = new DHBox(vbox);
    QLabel* const channelLabel = new QLabel(hbox);
    channelLabel->setText(i18nc("@label: colcor properties", "Channel:"));

    m_channelCB                = new QComboBox(hbox);
    m_channelCB->addItem(i18nc("@item: colcor properties", "Luminosity"), QVariant(LuminosityChannel));
    m_channelCB->addItem(i18nc("@item: colcor properties", "Red"),        QVariant(RedChannel));
    m_channelCB->addItem(i18nc("@item: colcor properties", "Green"),      QVariant(GreenChannel));
    m_channelCB->addItem(i18nc("@item: colcor properties", "Blue"),       QVariant(BlueChannel));
    m_channelCB->addItem(i18nc("@item: colcor properties", "Alpha"),      QVariant(AlphaChannel));

    m_settingsView             = new CurvesSettings(vbox, &m_preview);

    DHBox* const hbox1         = new DHBox(vbox);

    QPushButton* const loadBt  = new QPushButton(i18nc("@action: open file dialog", "Load..."), hbox1);
    loadBt->setIcon(QIcon::fromTheme(QLatin1String("document-open")));
    loadBt->setToolTip(i18nc("@info: open file dialog", "Load all parameters from settings text file."));

    QLabel* const load         = new QLabel(hbox1);
    load->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QLabel* const space = new QLabel(vbox);
    vbox->setStretchFactor(space, 10);

    m_settingsWidget = vbox;

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    connect(m_channelCB, SIGNAL(activated(int)),
            this, SLOT(slotChannelChanged()));

    connect(loadBt, SIGNAL(clicked()),
            this, SLOT(slotSettingsLoad()));

    BatchTool::registerSettingsWidget();
}

void CurvesAdjust::slotChannelChanged()
{
    int index = m_channelCB->currentIndex();
    m_settingsView->setCurrentChannel((ChannelType)(m_channelCB->itemData(index).toInt()));
}

void CurvesAdjust::slotResetSettingsToDefault()
{
    // We need to call this method there to reset all curves points.
    // Curves values are cleaned with default settings passed after.
    m_settingsView->resetToDefault();
    BatchTool::slotResetSettingsToDefault();
}

BatchToolSettings CurvesAdjust::defaultSettings()
{
    BatchToolSettings prm;
    CurvesContainer defaultPrm = m_settingsView->defaultSettings();
    prm.insert(QLatin1String("curvesType"),                (int)defaultPrm.curvesType);
    prm.insert(QLatin1String("curvesDepth"),               defaultPrm.sixteenBit);
    prm.insert(QLatin1String("values[LuminosityChannel]"), defaultPrm.values[LuminosityChannel]);
    prm.insert(QLatin1String("values[RedChannel]"),        defaultPrm.values[RedChannel]);
    prm.insert(QLatin1String("values[GreenChannel]"),      defaultPrm.values[GreenChannel]);
    prm.insert(QLatin1String("values[BlueChannel]"),       defaultPrm.values[BlueChannel]);
    prm.insert(QLatin1String("values[AlphaChannel]"),      defaultPrm.values[AlphaChannel]);

    return prm;
}

void CurvesAdjust::slotAssignSettings2Widget()
{
    CurvesContainer prm;

    prm.curvesType                = (ImageCurves::CurveType)settings()[QLatin1String("curvesType")].toInt();
    prm.sixteenBit                = settings()[QLatin1String("curvesDepth")].toBool();
    prm.values[LuminosityChannel] = settings()[QLatin1String("values[LuminosityChannel]")].value<QPolygon>();
    prm.values[RedChannel]        = settings()[QLatin1String("values[RedChannel]")].value<QPolygon>();
    prm.values[GreenChannel]      = settings()[QLatin1String("values[GreenChannel]")].value<QPolygon>();
    prm.values[BlueChannel]       = settings()[QLatin1String("values[BlueChannel]")].value<QPolygon>();
    prm.values[AlphaChannel]      = settings()[QLatin1String("values[AlphaChannel]")].value<QPolygon>();

    m_settingsView->setSettings(prm);
}

void CurvesAdjust::slotSettingsChanged()
{
    BatchToolSettings prm;
    CurvesContainer currentPrm = m_settingsView->settings();

    prm.insert(QLatin1String("curvesType"),                (int)currentPrm.curvesType);
    prm.insert(QLatin1String("curvesDepth"),               currentPrm.sixteenBit);
    prm.insert(QLatin1String("values[LuminosityChannel]"), currentPrm.values[LuminosityChannel]);
    prm.insert(QLatin1String("values[RedChannel]"),        currentPrm.values[RedChannel]);
    prm.insert(QLatin1String("values[GreenChannel]"),      currentPrm.values[GreenChannel]);
    prm.insert(QLatin1String("values[BlueChannel]"),       currentPrm.values[BlueChannel]);
    prm.insert(QLatin1String("values[AlphaChannel]"),      currentPrm.values[AlphaChannel]);

    BatchTool::slotSettingsChanged(prm);
}

void CurvesAdjust::slotSettingsLoad()
{
    m_settingsView->loadSettings();
    slotSettingsChanged();
}

bool CurvesAdjust::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    CurvesContainer prm((ImageCurves::CurveType)settings()[QLatin1String("curvesType")].toInt(),
                        settings()[QLatin1String("curvesDepth")].toBool());
    prm.initialize();
    prm.values[LuminosityChannel] = settings()[QLatin1String("values[LuminosityChannel]")].value<QPolygon>();
    prm.values[RedChannel]        = settings()[QLatin1String("values[RedChannel]")].value<QPolygon>();
    prm.values[GreenChannel]      = settings()[QLatin1String("values[GreenChannel]")].value<QPolygon>();
    prm.values[BlueChannel]       = settings()[QLatin1String("values[BlueChannel]")].value<QPolygon>();
    prm.values[AlphaChannel]      = settings()[QLatin1String("values[AlphaChannel]")].value<QPolygon>();

    CurvesFilter curves(&image(), nullptr, prm);
    applyFilter(&curves);

    return (savefromDImg());
}

} // namespace DigikamBqmCurvesAdjustPlugin
