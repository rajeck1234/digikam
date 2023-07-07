/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-16
 * Description : Dialog to adjust soft proofing settings
 *
 * SPDX-FileCopyrightText: 2009-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "softproofdialog.h"

// Qt includes

#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QIcon>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "iccprofilescombobox.h"
#include "iccsettingscontainer.h"
#include "iccsettings.h"
#include "iccprofileinfodlg.h"
#include "dexpanderbox.h"
#include "dcolorselector.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class Q_DECL_HIDDEN SoftProofDialog::Private
{
public:

    explicit Private()
      : switchOn         (false),
        deviceProfileBox (nullptr),
        infoProofProfiles(nullptr),
        buttons          (nullptr),
        gamutCheckBox    (nullptr),
        maskColorLabel   (nullptr),
        maskColorBtn     (nullptr),
        proofingIntentBox(nullptr)
    {
    }

    bool                        switchOn;

    IccProfilesComboBox*        deviceProfileBox;
    QPushButton*                infoProofProfiles;
    QDialogButtonBox*           buttons;
    QCheckBox*                  gamutCheckBox;
    QLabel*                     maskColorLabel;
    DColorSelector*             maskColorBtn;

    IccRenderingIntentComboBox* proofingIntentBox;
};

SoftProofDialog::SoftProofDialog(QWidget* const parent)
    : QDialog(parent),
      d      (new Private)
{
    setModal(true);
    setWindowTitle(i18nc("@title:window", "Soft Proofing Options"));

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setText(i18n("Soft Proofing On"));
    d->buttons->button(QDialogButtonBox::Ok)->setToolTip(i18n("Enable soft-proofing color managed view"));
    d->buttons->button(QDialogButtonBox::Cancel)->setText(i18n("Soft Proofing Off"));
    d->buttons->button(QDialogButtonBox::Cancel)->setToolTip(i18n("Disable soft-proofing color managed view"));

    QWidget* const page           = new QWidget(this);
    QVBoxLayout* const mainLayout = new QVBoxLayout(page);

    // ---

    QLabel* const headerLabel    = new QLabel(i18n("<b>Configure the Soft Proofing View</b>"));
    DLineWidget* const separator = new DLineWidget(Qt::Horizontal);

    // -------------------------------------------------------------

    QGridLayout* const profileGrid = new QGridLayout;
    QLabel* const proofIcon        = new QLabel;
    proofIcon->setPixmap(QIcon::fromTheme(QLatin1String("document-print")).pixmap(22));
    QLabel* const proofLabel       = new QLabel(i18n("Profile of the output device to simulate:"));
    d->deviceProfileBox            = new IccProfilesComboBox;
    proofLabel->setBuddy(d->deviceProfileBox);
    d->deviceProfileBox->setWhatsThis(i18n("<p>Select the profile for your output device "
                                           "(usually, your printer). This profile will be used to do a soft proof, so you will "
                                           "be able to preview how an image will be rendered via an output device.</p>"));

    d->infoProofProfiles      = new QPushButton;
    d->infoProofProfiles->setIcon(QIcon::fromTheme(QLatin1String("dialog-information")));
    d->infoProofProfiles->setWhatsThis(i18n("<p>Press this button to get detailed "
                                            "information about the selected proofing profile.</p>"));

    d->deviceProfileBox->replaceProfilesSqueezed(IccSettings::instance()->outputProfiles());

    profileGrid->addWidget(proofIcon,            0, 0);
    profileGrid->addWidget(proofLabel,           0, 1, 1, 2);
    profileGrid->addWidget(d->deviceProfileBox,  1, 0, 1, 2);
    profileGrid->addWidget(d->infoProofProfiles, 1, 2);
    profileGrid->setColumnStretch(1, 10);

    // --------------------------------------------------------------

    QGroupBox* const optionsBox    = new QGroupBox;
    QGridLayout* const optionsGrid = new QGridLayout;

    QLabel* const intentLabel      = new QLabel(i18n("Rendering intent:"));
    d->proofingIntentBox           = new IccRenderingIntentComboBox;

    intentLabel->setBuddy(d->proofingIntentBox);

    d->gamutCheckBox   = new QCheckBox(i18n("Highlight out-of-gamut colors"));
    d->maskColorLabel  = new QLabel(i18n("Highlighting color:"));
    d->maskColorBtn    = new DColorSelector;
    d->maskColorLabel->setBuddy(d->maskColorBtn);

    optionsGrid->addWidget(intentLabel,          0, 0, 1, 2);
    optionsGrid->addWidget(d->proofingIntentBox, 0, 2, 1, 2);
    optionsGrid->addWidget(d->gamutCheckBox,     1, 0, 1, 4);
    optionsGrid->addWidget(d->maskColorLabel,    2, 1, 1, 1);
    optionsGrid->addWidget(d->maskColorBtn,   2, 2, 1, 2, Qt::AlignLeft);
    optionsGrid->setColumnMinimumWidth(0, 10);
    optionsGrid->setColumnStretch(2, 1);
    optionsBox->setLayout(optionsGrid);

    // -------------------------------------------------------

    mainLayout->addWidget(headerLabel);
    mainLayout->addWidget(separator);
    mainLayout->addLayout(profileGrid);
    mainLayout->addWidget(optionsBox);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(page);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    connect(d->deviceProfileBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateOkButtonState()));

    connect(d->gamutCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(updateGamutCheckState()));

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotOk()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    connect(d->infoProofProfiles, SIGNAL(clicked()),
            this, SLOT(slotProfileInfo()));

    readSettings();
    updateOkButtonState();
    updateGamutCheckState();
}

SoftProofDialog::~SoftProofDialog()
{
    delete d;
}

bool SoftProofDialog::shallEnableSoftProofView() const
{
    return d->switchOn;
}

void SoftProofDialog::updateGamutCheckState()
{
    bool on = d->gamutCheckBox->isChecked();
    d->maskColorLabel->setEnabled(on);
    d->maskColorBtn->setEnabled(on);
}

void SoftProofDialog::updateOkButtonState()
{
    d->buttons->button(QDialogButtonBox::Ok)->setEnabled(d->deviceProfileBox->currentIndex() != -1);
}

void SoftProofDialog::readSettings()
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();
    d->deviceProfileBox->setCurrentProfile(IccProfile(settings.defaultProofProfile));
    d->proofingIntentBox->setIntent(settings.proofingRenderingIntent);
    d->gamutCheckBox->setChecked(settings.doGamutCheck);
    d->maskColorBtn->setColor(settings.gamutCheckMaskColor);
}

void SoftProofDialog::writeSettings()
{
    ICCSettingsContainer settings    = IccSettings::instance()->settings();
    settings.defaultProofProfile     = d->deviceProfileBox->currentProfile().filePath();
    settings.proofingRenderingIntent = d->proofingIntentBox->intent();
    settings.doGamutCheck            = d->gamutCheckBox->isChecked();
    settings.gamutCheckMaskColor     = d->maskColorBtn->color();
    IccSettings::instance()->setSettings(settings);
}

void SoftProofDialog::slotProfileInfo()
{
    IccProfile profile = d->deviceProfileBox->currentProfile();

    if (profile.isNull())
    {
        QMessageBox::critical(this, i18nc("@title:window", "Profile Error"), i18n("No profile is selected."));
        return;
    }

    ICCProfileInfoDlg infoDlg(this, profile.filePath(), profile);
    infoDlg.exec();
}

void SoftProofDialog::slotOk()
{
    d->switchOn = true;
    writeSettings();
    accept();
}

void SoftProofDialog::slotHelp()
{
    openOnlineDocumentation(QLatin1String("color_management"), QLatin1String("printer_profiles"));
}

} // namespace Digikam
