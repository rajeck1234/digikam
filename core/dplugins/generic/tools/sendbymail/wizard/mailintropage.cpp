/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items by email.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "mailintropage.h"

// Qt includes

#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QIcon>
#include <QGroupBox>
#include <QGridLayout>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "digikam_globals.h"
#include "dlayoutbox.h"
#include "mailwizard.h"
#include "mailsettings.h"
#include "dbinarysearch.h"
#include "balsabinary.h"
#include "clawsmailbinary.h"
#include "kmailbinary.h"
#include "evolutionbinary.h"
#include "netscapebinary.h"
#include "outlookbinary.h"
#include "sylpheedbinary.h"
#include "thunderbirdbinary.h"

namespace DigikamGenericSendByMailPlugin
{

class Q_DECL_HIDDEN MailIntroPage::Private
{
public:

    explicit Private(QWizard* const dialog)
      : imageGetOption(nullptr),
        hbox(nullptr),
        wizard(nullptr),
        iface(nullptr),
        binSearch(nullptr)
    {
        wizard = dynamic_cast<MailWizard*>(dialog);

        if (wizard)
        {
            iface = wizard->iface();
        }
    }

    QComboBox*        imageGetOption;
    DHBox*            hbox;
    MailWizard*       wizard;
    DInfoInterface*   iface;
    DBinarySearch*    binSearch;
    BalsaBinary       balsaBin;
    ClawsMailBinary   clawsBin;
    EvolutionBinary   evoluBin;
    KmailBinary       kmailBin;
    NetscapeBinary    netscBin;
    OutlookBinary     outloBin;
    SylpheedBinary    sylphBin;
    ThunderbirdBinary thundBin;
};

MailIntroPage::MailIntroPage(QWizard* const dialog, const QString& title)
    : DWizardPage(dialog, title),
      d(new Private(dialog))
{
    DVBox* const vbox  = new DVBox(this);
    QLabel* const desc = new QLabel(vbox);

    desc->setWordWrap(true);
    desc->setOpenExternalLinks(true);
    desc->setText(i18n("<qt>"
                       "<p><h1><b>Welcome to Email Tool</b></h1></p>"
                       "<p>This assistant will guide you to send "
                       "your items with a mail client application.</p>"
                       "<p>Before to export contents, you will be able to adjust attachments "
                       "properties accordingly with your mail service capabilities.</p>"
                       "</qt>"));

    // ComboBox for image selection method

    d->hbox                     = new DHBox(vbox);
    QLabel* const getImageLabel = new QLabel(i18n("&Choose image selection method:"), d->hbox);
    d->imageGetOption           = new QComboBox(d->hbox);
    d->imageGetOption->insertItem(MailSettings::ALBUMS, i18n("Albums"));
    d->imageGetOption->insertItem(MailSettings::IMAGES, i18n("Images"));
    getImageLabel->setBuddy(d->imageGetOption);

    // --------------------

    QGroupBox* const binaryBox      = new QGroupBox(vbox);
    QGridLayout* const binaryLayout = new QGridLayout;
    binaryBox->setLayout(binaryLayout);
    binaryBox->setTitle(i18nc("@title:group", "Mail client application Binaries"));
    d->binSearch = new DBinarySearch(binaryBox);
    d->binSearch->addBinary(d->balsaBin);
    d->binSearch->addBinary(d->clawsBin);
    d->binSearch->addBinary(d->kmailBin);
    d->binSearch->addBinary(d->evoluBin);
    d->binSearch->addBinary(d->netscBin);
    d->binSearch->addBinary(d->outloBin);
    d->binSearch->addBinary(d->sylphBin);
    d->binSearch->addBinary(d->thundBin);

#ifdef Q_OS_MACOS

    // Std Macports install

    d->binSearch->addDirectory(QLatin1String("/opt/local/bin"));

    // digiKam Bundle PKG install

    d->binSearch->addDirectory(macOSBundlePrefix() + QLatin1String("bin"));

#endif

#ifdef Q_OS_WIN

    // FIXME : adjust paths

    d->binSearch->addDirectory(QLatin1String("C:/Program Files/"));

    d->binSearch->addDirectory(QLatin1String("C:/Program Files (x86)/"));

#endif

    vbox->setStretchFactor(desc,      2);
    vbox->setStretchFactor(d->hbox,   1);
    vbox->setStretchFactor(binaryBox, 3);

    setPageWidget(vbox);
    setLeftBottomPix(QIcon::fromTheme(QLatin1String("mail-client")));

    connect(d->binSearch, SIGNAL(signalBinariesFound(bool)),
            this, SLOT(slotBinariesFound()));
}

MailIntroPage::~MailIntroPage()
{
    delete d;
}

void MailIntroPage::initializePage()
{
    bool albumSupport = (d->iface && d->iface->supportAlbums());

    if (!albumSupport)
    {
        d->imageGetOption->setCurrentIndex(MailSettings::IMAGES);
        d->hbox->setEnabled(false);
    }
    else
    {
        d->imageGetOption->setCurrentIndex(d->wizard->settings()->selMode);
    }

    d->binSearch->allBinariesFound();
    slotBinariesFound();
}

bool MailIntroPage::validatePage()
{
    d->wizard->settings()->selMode = (MailSettings::Selection)d->imageGetOption->currentIndex();

    return true;
}

void MailIntroPage::slotBinariesFound()
{
    d->wizard->settings()->binPaths.insert(MailSettings::BALSA, d->balsaBin.isValid() ?
                                           d->balsaBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::CLAWSMAIL, d->clawsBin.isValid() ?
                                           d->clawsBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::EVOLUTION, d->evoluBin.isValid() ?
                                           d->evoluBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::KMAIL, d->kmailBin.isValid() ?
                                           d->kmailBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::NETSCAPE, d->netscBin.isValid() ?
                                           d->netscBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::OUTLOOK, d->outloBin.isValid() ?
                                           d->outloBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::SYLPHEED, d->sylphBin.isValid() ?
                                           d->sylphBin.path() : QString());

    d->wizard->settings()->binPaths.insert(MailSettings::THUNDERBIRD, d->thundBin.isValid() ?
                                           d->thundBin.path() : QString());

    Q_EMIT completeChanged();
}

bool MailIntroPage::isComplete() const
{
    QString val = d->wizard->settings()->binPaths.values().join(QString());
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << val;

    return (!val.isEmpty());
}

} // namespace DigikamGenericSendByMailPlugin
