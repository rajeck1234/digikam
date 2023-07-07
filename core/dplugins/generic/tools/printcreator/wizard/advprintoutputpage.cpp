/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "advprintoutputpage.h"

// Qt includes

#include <QIcon>
#include <QLabel>
#include <QUrl>
#include <QWidget>
#include <QApplication>
#include <QStyle>
#include <QCheckBox>
#include <QGridLayout>
#include <QComboBox>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "advprintwizard.h"
#include "dfileselector.h"
#include "filesaveconflictbox.h"

namespace DigikamGenericPrintCreatorPlugin
{

class Q_DECL_HIDDEN AdvPrintOutputPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : labelImagesFormat(nullptr),
        destUrl          (nullptr),
        conflictBox      (nullptr),
        imagesFormat     (nullptr),
        fileBrowserCB    (nullptr),
        wizard           (nullptr),
        settings         (nullptr)
    {
        wizard = dynamic_cast<AdvPrintWizard*>(dialog);

        if (wizard)
        {
            settings = wizard->settings();
        }
    }

    QLabel*              labelImagesFormat;
    DFileSelector*       destUrl;
    FileSaveConflictBox* conflictBox;
    QComboBox*           imagesFormat;
    QCheckBox*           fileBrowserCB;
    AdvPrintWizard*      wizard;
    AdvPrintSettings*    settings;
};

AdvPrintOutputPage::AdvPrintOutputPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d          (new Private(dialog))
{
    QWidget* const main  = new QWidget(this);

    // --------------------

    d->labelImagesFormat = new QLabel(main);
    d->labelImagesFormat->setWordWrap(false);
    d->labelImagesFormat->setText(i18n("Image Format:"));

    d->imagesFormat      = new QComboBox(main);
    d->imagesFormat->setEditable(false);
    d->imagesFormat->setWhatsThis(i18n("Select your preferred format to export printing as image."));

    QMap<AdvPrintSettings::ImageFormat, QString> map2                = AdvPrintSettings::imageFormatNames();
    QMap<AdvPrintSettings::ImageFormat, QString>::const_iterator it2 = map2.constBegin();

    while (it2 != map2.constEnd())
    {
        d->imagesFormat->addItem(it2.value(), (int)it2.key());
        ++it2;
    }

    d->labelImagesFormat->setBuddy(d->imagesFormat);

    // --------------------

    QLabel* const fileLabel = new QLabel(main);
    fileLabel->setWordWrap(false);
    fileLabel->setText(i18n("Destination folder:"));

    d->destUrl              = new DFileSelector(main);
    d->destUrl->setFileDlgMode(QFileDialog::Directory);
    d->destUrl->setFileDlgOptions(QFileDialog::ShowDirsOnly);
    d->destUrl->setFileDlgTitle(i18nc("@title:window", "Destination Folder"));
    d->destUrl->lineEdit()->setPlaceholderText(i18n("Output Destination Path"));
    fileLabel->setBuddy(d->destUrl);

    // --------------------

    QLabel* const outputLbl = new QLabel(main);
    outputLbl->setText(i18n("The image output file name will be generated automatically."));
    d->conflictBox          = new FileSaveConflictBox(main);

    // --------------------

    d->fileBrowserCB = new QCheckBox(main);
    d->fileBrowserCB->setText(i18n("Open in File Browser"));

    // --------------------

    QGridLayout* const grid = new QGridLayout(main);
    grid->setSpacing(qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                             QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing)));
    grid->addWidget(d->labelImagesFormat, 0, 0, 1, 1);
    grid->addWidget(d->imagesFormat,      0, 1, 1, 2);
    grid->addWidget(fileLabel,            1, 0, 1, 1);
    grid->addWidget(d->destUrl,           1, 1, 1, 1);
    grid->addWidget(outputLbl,            2, 0, 1, 2);
    grid->addWidget(d->conflictBox,       3, 0, 1, 2);
    grid->addWidget(d->fileBrowserCB,     4, 0, 1, 2);
    grid->setRowStretch(5, 10);

    setPageWidget(main);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("folder-image")));

    connect(d->destUrl->lineEdit(), SIGNAL(textEdited(QString)),
            this, SIGNAL(completeChanged()));

    connect(d->destUrl, SIGNAL(signalUrlSelected(QUrl)),
            this, SIGNAL(completeChanged()));
}

AdvPrintOutputPage::~AdvPrintOutputPage()
{
    delete d;
}

void AdvPrintOutputPage::initializePage()
{
    d->destUrl->setFileDlgPath(d->settings->outputDir.toLocalFile());
    d->conflictBox->setConflictRule(d->settings->conflictRule);
    d->fileBrowserCB->setChecked(d->settings->openInFileBrowser);
    d->imagesFormat->setCurrentIndex((int)d->settings->imageFormat);
}

bool AdvPrintOutputPage::validatePage()
{
    if (d->destUrl->fileDlgPath().isEmpty())
    {
        return false;
    }

    d->settings->outputDir         = QUrl::fromLocalFile(d->destUrl->fileDlgPath());
    d->settings->conflictRule      = d->conflictBox->conflictRule();
    d->settings->openInFileBrowser = d->fileBrowserCB->isChecked();
    d->settings->imageFormat       = AdvPrintSettings::ImageFormat(d->imagesFormat->currentIndex());

    return true;
}

bool AdvPrintOutputPage::isComplete() const
{
    return (!d->destUrl->fileDlgPath().isEmpty());
}

} // namespace DigikamGenericPrintCreatorPlugin
