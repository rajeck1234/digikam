/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "setupraw.h"

// Qt includes

#include <QGridLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QIcon>
#include <QPointer>

// KDE includes

#include <kconfiggroup.h>
#include <ksharedconfig.h>
#include <klocalizedstring.h>

// Local includes

#include "drawdecoding.h"
#include "drawdecoderwidget.h"
#include "dpluginloader.h"
#include "dpluginrawimport.h"
#include "dpluginaboutdlg.h"
#include "setupeditor.h"

namespace Digikam
{

class Q_DECL_HIDDEN SetupRaw::Private
{
public:

    explicit Private()
      : tab             (nullptr),
        behaviorPanel   (nullptr),
        settingsPanel   (nullptr),
        openSimple      (nullptr),
        openDefault     (nullptr),
        openTool        (nullptr),
        rawImportTool   (nullptr),
        toolAbout       (nullptr),
        rawSettings     (nullptr)
    {
    }

    static const QString  configGroupName;
    static const QString  configUseRawImportToolEntry;
    static const QString  configRawImportToolIidEntry;
    static const QString  nativeRawImportToolIid;

    QTabWidget*           tab;

    QWidget*              behaviorPanel;
    QWidget*              settingsPanel;

    QRadioButton*         openSimple;
    QRadioButton*         openDefault;
    QRadioButton*         openTool;

    QComboBox*            rawImportTool;
    QPushButton*          toolAbout;

    DRawDecoderWidget*    rawSettings;
};

const QString SetupRaw::Private::configGroupName(QLatin1String("ImageViewer Settings"));
const QString SetupRaw::Private::configUseRawImportToolEntry(QLatin1String("UseRawImportTool"));
const QString SetupRaw::Private::configRawImportToolIidEntry(QLatin1String("RawImportToolIid"));
const QString SetupRaw::Private::nativeRawImportToolIid(QLatin1String("org.kde.digikam.plugin.rawimport.Native"));

SetupRaw::SetupRaw(QTabWidget* const tab)
    : QObject(tab),
      d      (new Private)
{
    d->tab = tab;

    // --------------------------------------------------------

    d->behaviorPanel                  = new QWidget;
    QVBoxLayout* const behaviorLayout = new QVBoxLayout;

    QLabel* const rawExplanation      = new QLabel;
    rawExplanation->setText(i18nc("@info",
                                  "A raw image file contains minimally processed data "
                                  "from the image sensor of a digital camera.\n"
                                  "Opening a raw file requires extensive data interpretation and processing."));
    rawExplanation->setWordWrap(true);
    QLabel* const rawIcon        = new QLabel;
    rawIcon->setPixmap(QIcon::fromTheme(QLatin1String("camera-photo")).pixmap(48));
    QHBoxLayout* const header    = new QHBoxLayout;
    header->addWidget(rawIcon);
    header->addWidget(rawExplanation);
    header->setStretchFactor(rawExplanation, 10);
    header->addStretch(1);

    QGroupBox* const behaviorBox = new QGroupBox;
    QGridLayout* const boxLayout = new QGridLayout;

    QLabel* const openIcon       = new QLabel;
    openIcon->setPixmap(QIcon::fromTheme(QLatin1String("document-open")).pixmap(32));

    QLabel* const openIntro      = new QLabel(i18nc("@label", "Open raw files in the image editor"));

    d->openSimple  = new QRadioButton(i18nc("@option:radio Open raw files...",
                                            "Fast and simple, as 8 bit image"));
    d->openDefault = new QRadioButton(i18nc("@option:radio Open raw files...",
                                            "Using the default settings, in 16 bit"));
    d->openTool    = new QRadioButton(i18nc("@option:radio Open raw files...",
                                            "Always open the Raw Import Tool to customize settings"));

    d->rawImportTool = new QComboBox;

    Q_FOREACH (DPlugin* const p, DPluginLoader::instance()->allPlugins())
    {
        DPluginRawImport* const raw = dynamic_cast<DPluginRawImport*>(p);

        if (raw)
        {
            QString iid  = raw->iid();
            QString name = raw->name();

            if (iid == d->nativeRawImportToolIid)
            {
                name += i18nc("@item: default raw plugin", " (default)");
            }

            d->rawImportTool->addItem(name, iid);
        }
    }

    d->toolAbout = new QPushButton;
    d->toolAbout->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
    d->toolAbout->setToolTip(i18nc("@info:tooltip", "About this Raw Import tool..."));

    // ----------------------------------------------

    boxLayout->addWidget(openIcon,         0, 0);
    boxLayout->addWidget(openIntro,        0, 1);
    boxLayout->addWidget(d->openSimple,    1, 0, 1, 4);
    boxLayout->addWidget(d->openDefault,   2, 0, 1, 4);
    boxLayout->addWidget(d->openTool,      3, 0, 1, 2);
    boxLayout->addWidget(d->rawImportTool, 3, 2, 1, 1);
    boxLayout->addWidget(d->toolAbout,     3, 3, 1, 1);
    boxLayout->setColumnStretch(2, 1);
    behaviorBox->setLayout(boxLayout);

    behaviorLayout->addLayout(header);
    behaviorLayout->addWidget(behaviorBox);
    behaviorLayout->addStretch();
    d->behaviorPanel->setLayout(behaviorLayout);

    // --------------------------------------------------------

    d->settingsPanel                  = new QWidget;
    QVBoxLayout* const settingsLayout = new QVBoxLayout;

    d->rawSettings                    = new DRawDecoderWidget(nullptr, 0 /* no advanced settings shown */);
    d->rawSettings->setItemIcon(0, QIcon::fromTheme(QLatin1String("image-x-adobe-dng")));
    d->rawSettings->setItemIcon(1, QIcon::fromTheme(QLatin1String("bordertool")));
    d->rawSettings->setItemIcon(2, QIcon::fromTheme(QLatin1String("lensdistortion")));

    settingsLayout->addWidget(d->rawSettings);
    d->settingsPanel->setLayout(settingsLayout);

    // --------------------------------------------------------

    d->tab->insertTab(SetupEditor::RAWBehavior,        d->behaviorPanel, i18nc("@title:tab", "RAW Behavior"));
    d->tab->insertTab(SetupEditor::RAWDefaultSettings, d->settingsPanel, i18nc("@title:tab", "RAW Default Settings"));

    // --------------------------------------------------------

    connect(d->openSimple, SIGNAL(toggled(bool)),
            this, SLOT(slotBehaviorChanged()));

    connect(d->openDefault, SIGNAL(toggled(bool)),
            this, SLOT(slotBehaviorChanged()));

    connect(d->openTool, SIGNAL(toggled(bool)),
            this, SLOT(slotBehaviorChanged()));

    connect(d->rawSettings, SIGNAL(signalSixteenBitsImageToggled(bool)),
            this, SLOT(slotSixteenBitsImageToggled(bool)));

    connect(d->toolAbout, SIGNAL(clicked()),
            this, SLOT(slotAboutRawImportPlugin()));

    // --------------------------------------------------------

    readSettings();
}

SetupRaw::~SetupRaw()
{
    delete d;
}

void SetupRaw::slotSixteenBitsImageToggled(bool)
{
    // Libraw provide a way to set brightness of image in 16 bits color depth.
    // We always set on this option. We drive brightness adjustment in digiKam Raw image loader.

    d->rawSettings->setEnabledBrightnessSettings(true);
}

void SetupRaw::slotBehaviorChanged()
{
    d->rawImportTool->setEnabled(d->openTool->isChecked());

    DRawDecoderSettings settings = d->rawSettings->settings();
    settings.sixteenBitsImage    = !d->openSimple->isChecked();
    d->rawSettings->setSettings(settings);
}

void SetupRaw::applySettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configUseRawImportToolEntry, d->openTool->isChecked());
    group.writeEntry(d->configRawImportToolIidEntry, d->rawImportTool->itemData(d->rawImportTool->currentIndex()));

    d->rawSettings->writeSettings(group);

    config->sync();
}

void SetupRaw::readSettings()
{
    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(d->configGroupName);

    d->rawSettings->readSettings(group);

    bool useTool = group.readEntry(d->configUseRawImportToolEntry, false);

    if (useTool)
    {
        d->openTool->setChecked(true);
    }
    else
    {
        if (d->rawSettings->settings().sixteenBitsImage)
        {
            d->openDefault->setChecked(true);
        }
        else
        {
            d->openSimple->setChecked(true);
        }
    }

    QString iid = group.readEntry(d->configRawImportToolIidEntry, d->nativeRawImportToolIid);
    d->rawImportTool->setCurrentIndex(d->rawImportTool->findData(iid));
}

void SetupRaw::slotAboutRawImportPlugin()
{
    QString iid = d->rawImportTool->itemData(d->rawImportTool->currentIndex()).toString();

    Q_FOREACH (DPlugin* const p, DPluginLoader::instance()->allPlugins())
    {
        DPluginRawImport* const raw = dynamic_cast<DPluginRawImport*>(p);

        if (raw)
        {
            if (raw->iid() == iid)
            {
                QPointer<DPluginAboutDlg> dlg = new DPluginAboutDlg(p);
                dlg->exec();
                delete dlg;
            }
        }
    }
}

} // namespace Digikam
