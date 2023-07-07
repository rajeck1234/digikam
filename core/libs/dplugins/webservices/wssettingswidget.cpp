/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-07-28
 * Description : Common widgets shared by Web Service tools
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2016-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "wssettingswidget.h"

// Qt includes

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QApplication>
#include <QComboBox>
#include <QScrollArea>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class Q_DECL_HIDDEN WSSettingsWidget::Private
{
public:

    explicit Private(DInfoInterface* const interface, const QString& name)
        : imgList           (nullptr),
          uploadWidget      (nullptr),
          toolName          (name),
          headerLbl         (nullptr),
          userNameDisplayLbl(nullptr),
          changeUserBtn     (nullptr),
          dlDimensionCoB    (nullptr),
          settingsScrollArea(nullptr),
          albumsCoB         (nullptr),
          newAlbumBtn       (nullptr),
          reloadAlbumsBtn   (nullptr),
          originalChB       (nullptr),
          resizeChB         (nullptr),
          photoIdChB        (nullptr),
          dimensionSpB      (nullptr),
          imageQualitySpB   (nullptr),
          mainLayout        (nullptr),
          settingsBox       (nullptr),
          settingsBoxLayout (nullptr),
          albBox            (nullptr),
          albumsBoxLayout   (nullptr),
          optionsBox        (nullptr),
          optionsBoxLayout  (nullptr),
          uploadBox         (nullptr),
          uploadBoxLayout   (nullptr),
          sizeBox           (nullptr),
          sizeBoxLayout     (nullptr),
          accountBox        (nullptr),
          accountBoxLayout  (nullptr),
          iface             (interface),
          progressBar       (nullptr)
    {
    }

    DItemsList*                   imgList;
    QWidget*                       uploadWidget;
    QString                        toolName;

    QLabel*                        headerLbl;
    QLabel*                        userNameDisplayLbl;
    QPushButton*                   changeUserBtn;
    QComboBox*                     dlDimensionCoB;
    QScrollArea*                   settingsScrollArea;

    QComboBox*                     albumsCoB;
    QPushButton*                   newAlbumBtn;
    QPushButton*                   reloadAlbumsBtn;

    QCheckBox*                     originalChB;
    QCheckBox*                     resizeChB;
    QCheckBox*                     photoIdChB;
    QSpinBox*                      dimensionSpB;
    QSpinBox*                      imageQualitySpB;

    QHBoxLayout*                   mainLayout;

    QWidget*                       settingsBox;
    QVBoxLayout*                   settingsBoxLayout;

    QGroupBox*                     albBox;
    QGridLayout*                   albumsBoxLayout;

    QGroupBox*                     optionsBox;
    QGridLayout*                   optionsBoxLayout;

    QGroupBox*                     uploadBox;
    QVBoxLayout*                   uploadBoxLayout;

    QGroupBox*                     sizeBox;
    QVBoxLayout*                   sizeBoxLayout;

    QGroupBox*                     accountBox;
    QGridLayout*                   accountBoxLayout;

    DInfoInterface*                iface;
    DProgressWdg*                  progressBar;
};

WSSettingsWidget::WSSettingsWidget(QWidget* const parent,
                                   DInfoInterface* const iface,
                                   const QString& toolName)
    : QWidget(parent),
      d      (new Private(iface, toolName))
{
    setObjectName(d->toolName + QLatin1String(" Widget"));

    d->mainLayout         = new QHBoxLayout(this);
    d->imgList            = new DItemsList(this);
    d->settingsScrollArea = new QScrollArea(this);
    d->settingsBox        = new QWidget(d->settingsScrollArea);
    d->settingsBoxLayout  = new QVBoxLayout(d->settingsBox);
    d->headerLbl          = new QLabel(this);
    d->accountBox         = new QGroupBox(i18n("Account"), d->settingsBox);
    d->accountBoxLayout   = new QGridLayout(d->accountBox);
    d->userNameDisplayLbl = new QLabel(d->accountBox);
    d->changeUserBtn      = new QPushButton(d->accountBox);
    d->albBox             = new QGroupBox(i18n("Album"), d->settingsBox);
    d->albumsBoxLayout    = new QGridLayout(d->albBox);
    d->albumsCoB          = new QComboBox(d->albBox);
    d->newAlbumBtn        = new QPushButton(d->accountBox);
    d->reloadAlbumsBtn    = new QPushButton(d->accountBox);
    d->sizeBox            = new QGroupBox(i18n("Max Dimension"), d->settingsBox);
    d->sizeBoxLayout      = new QVBoxLayout(d->sizeBox);
    d->dlDimensionCoB     = new QComboBox(d->sizeBox);
    d->uploadBox          = new QGroupBox(i18n("Destination"), d->settingsBox);
    d->uploadWidget       = iface->uploadWidget(d->uploadBox);
    d->uploadBoxLayout    = new QVBoxLayout(d->uploadBox);
    d->optionsBox         = new QGroupBox(i18n("Options"), d->settingsBox);
    d->optionsBoxLayout   = new QGridLayout(d->optionsBox);
    d->originalChB        = new QCheckBox(d->optionsBox);
    d->resizeChB          = new QCheckBox(d->optionsBox);
    d->photoIdChB         = new QCheckBox(d->optionsBox);
    d->dimensionSpB       = new QSpinBox(d->optionsBox);
    d->imageQualitySpB    = new QSpinBox(d->optionsBox);
    d->progressBar        = new DProgressWdg(this);

    //----------------------------------------------------------

    const int spacing = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    d->imgList->setObjectName(QLatin1String("WebService ImagesList"));

    d->imgList->appendControlButtonsWidget(d->progressBar);
    QBoxLayout* const blay = d->imgList->setControlButtonsPlacement(DItemsList::ControlButtonsBelow);
    blay->setStretchFactor(d->progressBar, 20);

    d->imgList->setAllowRAW(true);
    d->imgList->listView()->setWhatsThis(i18n("This is the list of images "
                                              "to upload to your %1 account.", d->toolName));
    d->imgList->setIface(d->iface);
    d->imgList->loadImagesFromCurrentSelection();

    d->settingsScrollArea->setMinimumSize(400, 500);
    d->settingsScrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    d->settingsScrollArea->setWidget(d->settingsBox);
    d->settingsScrollArea->setWidgetResizable(true);
    d->settingsScrollArea->setFrameShadow(QFrame::Plain);

    d->headerLbl->setWhatsThis(i18n("This is a clickable link to open %1 in a browser.", d->toolName));
    d->headerLbl->setOpenExternalLinks(true);
    d->headerLbl->setFocusPolicy(Qt::NoFocus);

    //------------------------------------------------------------

    d->accountBox->setWhatsThis(i18n("This is the %1 account that is currently logged in.", d->toolName));

    QLabel* const userNameLbl = new QLabel(i18nc("account settings","Name:"), d->accountBox);
    d->changeUserBtn->setText(i18n("Change Account"));
    d->changeUserBtn->setIcon(QIcon::fromTheme(QLatin1String("system-switch-user")).pixmap(16));
    d->changeUserBtn->setToolTip(i18n("Change %1 account for transfer", d->toolName));

    d->accountBoxLayout->addWidget(userNameLbl,           0, 0, 1, 2);
    d->accountBoxLayout->addWidget(d->userNameDisplayLbl, 0, 2, 1, 2);
    d->accountBoxLayout->addWidget(d->changeUserBtn,      1, 0, 1, 4);
    d->accountBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    d->accountBoxLayout->setSpacing(spacing);

    //-------------------------------------------------------------

    d->albBox->setWhatsThis(i18n("This is the %1 folder to/from which selected "
                                 "photos will be uploaded/downloaded.", d->toolName));

    QLabel* const albLbl = new QLabel(i18n("Album:"), d->albBox);

    d->albumsCoB->setEditable(false);

    d->newAlbumBtn->setText(i18n("New Album"));
    d->newAlbumBtn->setIcon(QIcon::fromTheme(QLatin1String("list-add")).pixmap(16));
    d->newAlbumBtn->setToolTip(i18n("Create new %1 folder", d->toolName));

    d->reloadAlbumsBtn->setText(i18nc("album list","Reload"));
    d->reloadAlbumsBtn->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")).pixmap(16));
    d->reloadAlbumsBtn->setToolTip(i18n("Reload album list"));

    d->albumsBoxLayout->addWidget(albLbl,             0, 0, 1, 1);
    d->albumsBoxLayout->addWidget(d->albumsCoB,       0, 1, 1, 4);
    d->albumsBoxLayout->addWidget(d->newAlbumBtn,     1, 3, 1, 1);
    d->albumsBoxLayout->addWidget(d->reloadAlbumsBtn, 1, 4, 1, 1);

    //----------------------------------------------------------

    d->sizeBox->setWhatsThis(i18n("This is the maximum dimension of the images. "
                                  "Images larger than this will be scaled down."));
    d->dlDimensionCoB->addItem(i18n("Original Size"), QLatin1String("d"));
    d->dlDimensionCoB->addItem(i18n("1600 px"), QLatin1String("1600"));
    d->dlDimensionCoB->addItem(i18n("1440 px"), QLatin1String("1440"));
    d->dlDimensionCoB->addItem(i18n("1280 px"), QLatin1String("1280"));
    d->dlDimensionCoB->addItem(i18n("1152 px"), QLatin1String("1152"));
    d->dlDimensionCoB->addItem(i18n("1024 px"), QLatin1String("1024"));
    d->dlDimensionCoB->setCurrentIndex(0);
    d->sizeBoxLayout->addWidget(d->dlDimensionCoB);

    // ------------------------------------------------------------------------

    d->uploadBox->setWhatsThis(i18n("This is the location where %1 "
                                    "images will be downloaded.", d->toolName));
    d->uploadBoxLayout->addWidget(d->uploadWidget);

    //-----------------------------------------------------------

    d->optionsBox->setWhatsThis(i18n("These are the options that would "
                                     "be applied to photos before upload."));

    d->originalChB->setText(i18n("Upload original image file"));
    d->originalChB->setChecked(false);
    d->originalChB->hide();

    d->resizeChB->setText(i18n("Resize photos before uploading"));
    d->resizeChB->setChecked(false);

    d->photoIdChB->setText(i18n("Write the photo ID to the source image"));
    d->photoIdChB->setWhatsThis(i18n("Write the photo ID as metadata in "
                                     "the source image after uploading."));
    d->photoIdChB->setChecked(false);
    d->photoIdChB->hide();

    d->dimensionSpB->setMinimum(0);
    d->dimensionSpB->setMaximum(5000);
    d->dimensionSpB->setSingleStep(10);
    d->dimensionSpB->setValue(1600);
    d->dimensionSpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    d->dimensionSpB->setEnabled(false);

    QLabel* const dimensionLbl = new QLabel(i18n("Maximum Dimension:"), d->optionsBox);

    d->imageQualitySpB->setMinimum(0);
    d->imageQualitySpB->setMaximum(100);
    d->imageQualitySpB->setSingleStep(1);
    d->imageQualitySpB->setValue(90);
    d->imageQualitySpB->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* const imageQualityLbl = new QLabel(i18n("JPEG Quality:"), d->optionsBox);

    d->optionsBoxLayout->addWidget(d->originalChB,     0, 0, 1, 5);
    d->optionsBoxLayout->addWidget(d->resizeChB,       1, 0, 1, 5);
    d->optionsBoxLayout->addWidget(imageQualityLbl,    2, 1, 1, 1);
    d->optionsBoxLayout->addWidget(d->imageQualitySpB, 2, 2, 1, 1);
    d->optionsBoxLayout->addWidget(dimensionLbl,       3, 1, 1, 1);
    d->optionsBoxLayout->addWidget(d->dimensionSpB,    3, 2, 1, 1);
    d->optionsBoxLayout->addWidget(d->photoIdChB,      4, 0, 1, 5);
    d->optionsBoxLayout->setRowStretch(5, 10);
    d->optionsBoxLayout->setSpacing(spacing);
    d->optionsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);

    d->progressBar->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->progressBar->hide();

    //------------------------------------------------------

    d->settingsBoxLayout->addWidget(d->headerLbl);
    d->settingsBoxLayout->addWidget(d->accountBox);
    d->settingsBoxLayout->addWidget(d->albBox);
    d->settingsBoxLayout->addWidget(d->sizeBox);
    d->settingsBoxLayout->addWidget(d->uploadBox);
    d->settingsBoxLayout->addWidget(d->optionsBox);
    d->settingsBoxLayout->setSpacing(spacing);
    d->settingsBoxLayout->setContentsMargins(spacing, spacing, spacing, spacing);

    //--------------------------------------------------------

    d->mainLayout->addWidget(d->imgList);
    d->mainLayout->addWidget(d->settingsScrollArea);
    d->mainLayout->setContentsMargins(QMargins());
    d->mainLayout->setSpacing(spacing);

    //-------------------------------------------------------

    connect(d->originalChB, SIGNAL(toggled(bool)),
            this, SLOT(slotResizeChecked()));

    connect(d->resizeChB, SIGNAL(toggled(bool)),
            this, SLOT(slotResizeChecked()));
}

WSSettingsWidget::~WSSettingsWidget()
{
    delete d;
}

QString WSSettingsWidget::getDestinationPath() const
{
    QUrl url = d->iface->uploadUrl();
    return url.toLocalFile();
}

DItemsList* WSSettingsWidget::imagesList() const
{
    return d->imgList;
}

void WSSettingsWidget::slotResizeChecked()
{
    d->resizeChB->setEnabled(!d->originalChB->isChecked());
    d->imageQualitySpB->setEnabled(!d->originalChB->isChecked());
    d->dimensionSpB->setEnabled(d->resizeChB->isChecked() && !d->originalChB->isChecked());
}

DProgressWdg* WSSettingsWidget::progressBar() const
{
    return d->progressBar;
}

void WSSettingsWidget::addWidgetToSettingsBox(QWidget* const widget)
{
    d->settingsBoxLayout->addWidget(widget);

    // NOTE: This is important because progress bar always has to be at the end of settings box layout.
    // So we remove it and then add it back.

    d->settingsBoxLayout->removeWidget(d->progressBar);

    d->settingsBoxLayout->addWidget(d->progressBar);
}

void WSSettingsWidget::replaceImageList(QWidget* const imgList)
{
    d->imgList->hide();
    d->mainLayout->removeWidget(d->imgList);
    d->mainLayout->insertWidget(0, imgList);
}

QWidget* WSSettingsWidget::getSettingsBox() const
{
    return d->settingsBox;
}

QVBoxLayout* WSSettingsWidget::getSettingsBoxLayout() const
{
    return d->settingsBoxLayout;
}

QGroupBox* WSSettingsWidget::getAlbumBox() const
{
    return d->albBox;
}

QGridLayout* WSSettingsWidget::getAlbumBoxLayout() const
{
    return d->albumsBoxLayout;
}

QGroupBox* WSSettingsWidget::getOptionsBox() const
{
    return d->optionsBox;
}

QGridLayout* WSSettingsWidget::getOptionsBoxLayout() const
{
    return d->optionsBoxLayout;
}

QGroupBox* WSSettingsWidget::getUploadBox() const
{
    return d->uploadBox;
}

QVBoxLayout* WSSettingsWidget::getUploadBoxLayout() const
{
    return d->uploadBoxLayout;
}

QGroupBox* WSSettingsWidget::getSizeBox() const
{
    return d->sizeBox;
}

QVBoxLayout* WSSettingsWidget::getSizeBoxLayout() const
{
    return d->sizeBoxLayout;
}

QGroupBox* WSSettingsWidget::getAccountBox() const
{
    return d->accountBox;
}

QGridLayout* WSSettingsWidget::getAccountBoxLayout() const
{
    return d->accountBoxLayout;
}

QLabel* WSSettingsWidget::getHeaderLbl() const
{
    return d->headerLbl;
}

QLabel* WSSettingsWidget::getUserNameLabel() const
{
    return d->userNameDisplayLbl;
}

QPushButton* WSSettingsWidget::getChangeUserBtn() const
{
    return d->changeUserBtn;
}

QComboBox* WSSettingsWidget::getDimensionCoB() const
{
    return d->dlDimensionCoB;
}

QPushButton* WSSettingsWidget::getNewAlbmBtn() const
{
    return d->newAlbumBtn;
}

QPushButton* WSSettingsWidget::getReloadBtn() const
{
    return d->reloadAlbumsBtn;
}

QCheckBox* WSSettingsWidget::getOriginalCheckBox() const
{
    return d->originalChB;
}

QCheckBox* WSSettingsWidget::getResizeCheckBox() const
{
    return d->resizeChB;
}

QCheckBox* WSSettingsWidget::getPhotoIdCheckBox() const
{
    return d->photoIdChB;
}

QSpinBox* WSSettingsWidget::getDimensionSpB() const
{
    return d->dimensionSpB;
}

QSpinBox* WSSettingsWidget::getImgQualitySpB() const
{
    return d->imageQualitySpB;
}

QComboBox* WSSettingsWidget::getAlbumsCoB() const
{
    return d->albumsCoB;
}

} // namespace Digikam
