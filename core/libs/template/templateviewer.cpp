/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-29
 * Description : metadata template viewer.
 *
 * SPDX-FileCopyrightText: 2009-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "templateviewer.h"

// Qt includes

#include <QGridLayout>
#include <QUrl>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "itempropertiestxtlabel.h"
#include "template.h"
#include "templatemanager.h"
#include "countryselector.h"

namespace Digikam
{

class Q_DECL_HIDDEN TemplateViewer::Private
{
public:

    explicit Private()
      : names                       (nullptr),
        position                    (nullptr),
        credit                      (nullptr),
        copyright                   (nullptr),
        usages                      (nullptr),
        source                      (nullptr),
        instructions                (nullptr),
        namesList                   (nullptr),
        labelPosition               (nullptr),
        labelCredit                 (nullptr),
        labelCopyright              (nullptr),
        labelUsages                 (nullptr),
        labelSource                 (nullptr),
        labelInstructions           (nullptr),
        locationCountry             (nullptr),
        locationProvinceState       (nullptr),
        locationCity                (nullptr),
        locationSublocation         (nullptr),
        labelLocationCountry        (nullptr),
        labelLocationProvinceState  (nullptr),
        labelLocationCity           (nullptr),
        labelLocationSublocation    (nullptr),
        contactCity                 (nullptr),
        contactCountry              (nullptr),
        contactAddress              (nullptr),
        contactPostalCode           (nullptr),
        contactProvinceState        (nullptr),
        contactEmail                (nullptr),
        contactPhone                (nullptr),
        contactWebUrl               (nullptr),
        labelContactCity            (nullptr),
        labelContactCountry         (nullptr),
        labelContactAddress         (nullptr),
        labelContactPostalCode      (nullptr),
        labelContactProvinceState   (nullptr),
        labelContactPhone           (nullptr),
        labelContactEmail           (nullptr),
        labelContactWebUrl          (nullptr),
        subjectsList                (nullptr)
    {
    }

    /// IPTC Rights info.
    DTextLabelName*  names;
    DTextLabelName*  position;
    DTextLabelName*  credit;
    DTextLabelName*  copyright;
    DTextLabelName*  usages;
    DTextLabelName*  source;
    DTextLabelName*  instructions;

    DTextList*       namesList;
    DTextBrowser*    labelPosition;
    DTextBrowser*    labelCredit;
    DTextBrowser*    labelCopyright;
    DTextBrowser*    labelUsages;
    DTextBrowser*    labelSource;
    DTextBrowser*    labelInstructions;

    /// IPTC Location info.
    DTextLabelName*  locationCountry;
    DTextLabelName*  locationProvinceState;
    DTextLabelName*  locationCity;
    DTextLabelName*  locationSublocation;

    DTextBrowser*    labelLocationCountry;
    DTextBrowser*    labelLocationProvinceState;
    DTextBrowser*    labelLocationCity;
    DTextBrowser*    labelLocationSublocation;

    /// IPTC Contact info.
    DTextLabelName*  contactCity;
    DTextLabelName*  contactCountry;
    DTextLabelName*  contactAddress;
    DTextLabelName*  contactPostalCode;
    DTextLabelName*  contactProvinceState;
    DTextLabelName*  contactEmail;
    DTextLabelName*  contactPhone;
    DTextLabelName*  contactWebUrl;

    DTextBrowser*    labelContactCity;
    DTextBrowser*    labelContactCountry;
    DTextBrowser*    labelContactAddress;
    DTextBrowser*    labelContactPostalCode;
    DTextBrowser*    labelContactProvinceState;
    DTextBrowser*    labelContactPhone;
    QLabel*          labelContactEmail;
    QLabel*          labelContactWebUrl;

    /// IPTC Subjects info.
    DTextList*       subjectsList;
};

TemplateViewer::TemplateViewer(QWidget* const parent)
    : DExpanderBox(parent),
      d           (new Private)
{
    setFrameStyle(QFrame::NoFrame);

    DVBox* const w1      = new DVBox(this);
    d->names             = new DTextLabelName(i18n("Names:"), w1);
    d->namesList         = new DTextList(QStringList(), w1);
    d->position          = new DTextLabelName(i18n("Position:"), w1);
    d->labelPosition     = new DTextBrowser(QString(), w1);
    d->credit            = new DTextLabelName(i18n("Credit:"), w1);
    d->labelCredit       = new DTextBrowser(QString(), w1);
    d->copyright         = new DTextLabelName(i18n("Copyright:"), w1);
    d->labelCopyright    = new DTextBrowser(QString(), w1);
    d->usages            = new DTextLabelName(i18n("Usages:"), w1);
    d->labelUsages       = new DTextBrowser(QString(), w1);
    d->source            = new DTextLabelName(i18nc("@label: template properties", "Source:"), w1);
    d->labelSource       = new DTextBrowser(QString(), w1);
    d->instructions      = new DTextLabelName(i18n("Instructions:"), w1);
    d->labelInstructions = new DTextBrowser(QString(), w1);

    d->names->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->position->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->credit->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->copyright->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->credit->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->usages->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->source->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->instructions->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w1, QIcon::fromTheme(QLatin1String("flag")),
            i18n("Rights"), QLatin1String("Rights"), true);

    // ------------------------------------------------------------------

    DVBox* const w2               = new DVBox(this);
    d->locationCity               = new DTextLabelName(i18n("City:"), w2);
    d->labelLocationCity          = new DTextBrowser(QString(), w2);
    d->locationSublocation        = new DTextLabelName(i18n("Sublocation:"), w2);
    d->labelLocationSublocation   = new DTextBrowser(QString(), w2);
    d->locationProvinceState      = new DTextLabelName(i18n("State/Province:"), w2);
    d->labelLocationProvinceState = new DTextBrowser(QString(), w2);
    d->locationCountry            = new DTextLabelName(i18n("Country:"), w2);
    d->labelLocationCountry       = new DTextBrowser(QString(), w2);

    d->locationCity->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationSublocation->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationProvinceState->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->locationCountry->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w2, QIcon::fromTheme(QLatin1String("globe")),
            i18n("Location"), QLatin1String("Location"), true);

    // ------------------------------------------------------------------

    DVBox* const w3              = new DVBox(this);
    d->contactAddress            = new DTextLabelName(i18nc("@label: template properties", "Address:"), w3);
    d->labelContactAddress       = new DTextBrowser(QString(), w3);
    d->contactPostalCode         = new DTextLabelName(i18n("Postal Code:"), w3);
    d->labelContactPostalCode    = new DTextBrowser(QString(), w3);
    d->contactCity               = new DTextLabelName(i18n("City:"), w3);
    d->labelContactCity          = new DTextBrowser(QString(), w3);
    d->contactProvinceState      = new DTextLabelName(i18n("State/Province:"), w3);
    d->labelContactProvinceState = new DTextBrowser(QString(), w3);
    d->contactCountry            = new DTextLabelName(i18n("Country:"), w3);
    d->labelContactCountry       = new DTextBrowser(QString(), w3);
    d->contactPhone              = new DTextLabelName(i18n("Phone:"), w3);
    d->labelContactPhone         = new DTextBrowser(QString(), w3);
    d->contactEmail              = new DTextLabelName(i18nc("@label: template properties", "Email:"), w3);
    d->labelContactEmail         = new QLabel(QString(), w3);
    d->contactWebUrl             = new DTextLabelName(i18n("URL:"), w3);
    d->labelContactWebUrl        = new QLabel(QString(), w3);

    d->labelContactEmail->setOpenExternalLinks(true);
    d->labelContactEmail->setTextFormat(Qt::RichText);
    d->labelContactEmail->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    d->labelContactWebUrl->setOpenExternalLinks(true);
    d->labelContactWebUrl->setTextFormat(Qt::RichText);
    d->labelContactWebUrl->setTextInteractionFlags(Qt::LinksAccessibleByMouse);

    d->contactAddress->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactPostalCode->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactCity->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactProvinceState->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactCountry->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactPhone->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactEmail->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    d->contactWebUrl->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    addItem(w3, QIcon::fromTheme(QLatin1String("view-pim-contacts")),
            i18n("Contact"), QLatin1String("Contact"), true);

    // ------------------------------------------------------------------

    DVBox* const w4 = new DVBox(this);
    d->subjectsList = new DTextList(QStringList(), w4);

    addItem(w4, QIcon::fromTheme(QLatin1String("feed-subscribe")),
            i18n("Subjects"), QLatin1String("Subjects"), true);

    addStretch();
}

TemplateViewer::~TemplateViewer()
{
    delete d;
}

void TemplateViewer::setTemplate(const Template& t)
{
    d->namesList->clear();
    d->namesList->addItems(t.authors());
    d->labelPosition->setText(t.authorsPosition());
    d->labelCredit->setText(t.credit());
    d->labelCopyright->setText(t.copyright().value(QLatin1String("x-default")));
    d->labelUsages->setText(t.rightUsageTerms().value(QLatin1String("x-default")));
    d->labelSource->setText(t.source());
    d->labelInstructions->setText(t.instructions());

    d->labelLocationCity->setText(t.locationInfo().city);
    d->labelLocationSublocation->setText(t.locationInfo().location);
    d->labelLocationProvinceState->setText(t.locationInfo().provinceState);
    d->labelLocationCountry->setText(CountrySelector::countryForCode(t.locationInfo().countryCode));

    d->labelContactAddress->setText(t.contactInfo().address);
    d->labelContactPostalCode->setText(t.contactInfo().postalCode);
    d->labelContactProvinceState->setText(t.contactInfo().provinceState);
    d->labelContactCity->setText(t.contactInfo().city);
    d->labelContactCountry->setText(t.contactInfo().country);
    d->labelContactPhone->setText(t.contactInfo().phone);
    d->labelContactEmail->setText(QString::fromUtf8("<a href=\"mailto:%1\">%2</a>").arg(t.contactInfo().email).arg(t.contactInfo().email));
    d->labelContactWebUrl->setText(QString::fromUtf8("<a href=\"%1\">%2</a>").arg(t.contactInfo().webUrl).arg(t.contactInfo().webUrl));

    d->subjectsList->clear();
    d->subjectsList->addItems(t.IptcSubjects());
}

} // namespace Digikam
