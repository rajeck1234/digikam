/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-24
 * Description : XMP origin settings page.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "xmporigin.h"

// Qt includes

#include <QCheckBox>
#include <QMap>
#include <QPushButton>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QDateTimeEdit>
#include <QLineEdit>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "squeezedcombobox.h"
#include "dlayoutbox.h"
#include "metadatacheckbox.h"
#include "timezonecombobox.h"
#include "countryselector.h"
#include "dexpanderbox.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN XMPOrigin::Private
{
public:

    explicit Private()
      : dateCreatedCheck      (nullptr),
        dateDigitalizedCheck  (nullptr),
        dateVideoCheck        (nullptr),
        syncEXIFDateCheck     (nullptr),
        cityCheck             (nullptr),
        sublocationCheck      (nullptr),
        provinceCheck         (nullptr),
        setTodayCreatedBtn    (nullptr),
        setTodayDigitalizedBtn(nullptr),
        setTodayVideoBtn      (nullptr),
        dateCreatedSel        (nullptr),
        dateDigitalizedSel    (nullptr),
        dateVideoSel          (nullptr),
        zoneCreatedSel        (nullptr),
        zoneDigitalizedSel    (nullptr),
        zoneVideoSel          (nullptr),
        cityEdit              (nullptr),
        sublocationEdit       (nullptr),
        provinceEdit          (nullptr),
        countryCheck          (nullptr),
        countryCB             (nullptr)
    {
    }

    QCheckBox*                     dateCreatedCheck;
    QCheckBox*                     dateDigitalizedCheck;
    QCheckBox*                     dateVideoCheck;
    QCheckBox*                     syncEXIFDateCheck;
    QCheckBox*                     cityCheck;
    QCheckBox*                     sublocationCheck;
    QCheckBox*                     provinceCheck;

    QPushButton*                   setTodayCreatedBtn;
    QPushButton*                   setTodayDigitalizedBtn;
    QPushButton*                   setTodayVideoBtn;

    QDateTimeEdit*                 dateCreatedSel;
    QDateTimeEdit*                 dateDigitalizedSel;
    QDateTimeEdit*                 dateVideoSel;

    TimeZoneComboBox*              zoneCreatedSel;
    TimeZoneComboBox*              zoneDigitalizedSel;
    TimeZoneComboBox*              zoneVideoSel;

    QLineEdit*                     cityEdit;
    QLineEdit*                     sublocationEdit;
    QLineEdit*                     provinceEdit;

    MetadataCheckBox*              countryCheck;

    CountrySelector*               countryCB;
};

XMPOrigin::XMPOrigin(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());

    QString dateTimeFormat  = QLocale().dateTimeFormat(QLocale::ShortFormat);

    if (!dateTimeFormat.contains(QLatin1String("yyyy")))
    {
        dateTimeFormat.replace(QLatin1String("yy"),
                               QLatin1String("yyyy"));
    }

    if (!dateTimeFormat.contains(QLatin1String("ss")))
    {
        dateTimeFormat.replace(QLatin1String("mm"),
                               QLatin1String("mm:ss"));
    }

    // --------------------------------------------------------

    d->dateDigitalizedCheck   = new QCheckBox(i18n("Digitization date"), this);
    d->zoneDigitalizedSel     = new TimeZoneComboBox(this);

    d->dateDigitalizedSel     = new QDateTimeEdit(this);
    d->dateDigitalizedSel->setDisplayFormat(dateTimeFormat);

    d->setTodayDigitalizedBtn = new QPushButton();
    d->setTodayDigitalizedBtn->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));
    d->setTodayDigitalizedBtn->setWhatsThis(i18n("Set digitization date to today"));

    d->dateDigitalizedSel->setWhatsThis(i18n("Set here the creation date of "
                                             "digital representation."));
    d->zoneDigitalizedSel->setWhatsThis(i18n("Set here the time zone of "
                                             "digital representation."));

    slotSetTodayDigitalized();

    // --------------------------------------------------------

    d->dateCreatedCheck   = new QCheckBox(i18n("Creation date"), this);
    d->zoneCreatedSel     = new TimeZoneComboBox(this);

    d->dateCreatedSel     = new QDateTimeEdit(this);
    d->dateCreatedSel->setDisplayFormat(dateTimeFormat);

    d->syncEXIFDateCheck  = new QCheckBox(i18n("Sync Exif creation date"), this);

    d->setTodayCreatedBtn = new QPushButton();
    d->setTodayCreatedBtn->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));
    d->setTodayCreatedBtn->setWhatsThis(i18n("Set creation date to today"));

    d->dateCreatedSel->setWhatsThis(i18n("Set here the creation date of "
                                         "intellectual content."));
    d->zoneCreatedSel->setWhatsThis(i18n("Set here the time zone of "
                                         "intellectual content."));

    slotSetTodayCreated();

    // --------------------------------------------------------

    d->dateVideoCheck   = new QCheckBox(i18n("Video date"), this);
    d->zoneVideoSel     = new TimeZoneComboBox(this);

    d->dateVideoSel     = new QDateTimeEdit(this);
    d->dateVideoSel->setDisplayFormat(dateTimeFormat);

    d->setTodayVideoBtn = new QPushButton();
    d->setTodayVideoBtn->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));
    d->setTodayVideoBtn->setWhatsThis(i18n("Set video date to today"));

    d->dateVideoSel->setWhatsThis(i18n("Set here the video date of "
                                       "intellectual content."));
    d->zoneVideoSel->setWhatsThis(i18n("Set here the time zone of "
                                       "intellectual content."));

    slotSetTodayVideo();

    // --------------------------------------------------------

    d->cityCheck = new QCheckBox(i18n("City:"), this);
    d->cityEdit  = new QLineEdit(this);
    d->cityEdit->setClearButtonEnabled(true);
    d->cityEdit->setPlaceholderText(i18n("Set here the content's city of origin."));

    // --------------------------------------------------------

    d->sublocationCheck = new QCheckBox(i18n("Sublocation:"), this);
    d->sublocationEdit  = new QLineEdit(this);
    d->sublocationEdit->setClearButtonEnabled(true);
    d->sublocationEdit->setPlaceholderText(i18n("Set here the content's location within the city."));

    // --------------------------------------------------------

    d->provinceCheck = new QCheckBox(i18n("State/Province:"), this);
    d->provinceEdit  = new QLineEdit(this);
    d->provinceEdit->setClearButtonEnabled(true);
    d->provinceEdit->setPlaceholderText(i18n("Set here the content's Province or State of origin."));

    // --------------------------------------------------------

    d->countryCheck = new MetadataCheckBox(i18n("Country:"), this);
    d->countryCB    = new CountrySelector(this);
    d->countryCB->setWhatsThis(i18n("Select here country name of content origin."));

    // Remove 2 last items for the list (separator + Unknown item)

    d->countryCB->removeItem(d->countryCB->count()-1);
    d->countryCB->removeItem(d->countryCB->count()-1);

    // --------------------------------------------------------

    grid->addWidget(d->dateDigitalizedCheck,                0, 0, 1, 6);
    grid->addWidget(d->dateDigitalizedSel,                  1, 0, 1, 3);
    grid->addWidget(d->zoneDigitalizedSel,                  1, 3, 1, 1);
    grid->addWidget(d->setTodayDigitalizedBtn,              1, 5, 1, 1);
    grid->addWidget(d->dateCreatedCheck,                    2, 0, 1, 6);
    grid->addWidget(d->dateCreatedSel,                      3, 0, 1, 3);
    grid->addWidget(d->zoneCreatedSel,                      3, 3, 1, 1);
    grid->addWidget(d->setTodayCreatedBtn,                  3, 5, 1, 1);
    grid->addWidget(d->syncEXIFDateCheck,                   4, 0, 1, 6);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  5, 0, 1, 6);
    grid->addWidget(d->dateVideoCheck,                      6, 0, 1, 6);
    grid->addWidget(d->dateVideoSel,                        7, 0, 1, 3);
    grid->addWidget(d->zoneVideoSel,                        7, 3, 1, 1);
    grid->addWidget(d->setTodayVideoBtn,                    7, 5, 1, 1);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  8, 0, 1, 6);
    grid->addWidget(d->cityCheck,                           9, 0, 1, 1);
    grid->addWidget(d->cityEdit,                            9, 1, 1, 5);
    grid->addWidget(d->sublocationCheck,                   10, 0, 1, 1);
    grid->addWidget(d->sublocationEdit,                    10, 1, 1, 5);
    grid->addWidget(d->provinceCheck,                      11, 0, 1, 1);
    grid->addWidget(d->provinceEdit,                       11, 1, 1, 5);
    grid->addWidget(d->countryCheck,                       12, 0, 1, 1);
    grid->addWidget(d->countryCB,                          12, 1, 1, 5);
    grid->setColumnStretch(4, 10);
    grid->setRowStretch(13, 10);

    int spacing = qMin(QApplication::style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                       QApplication::style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    grid->setContentsMargins(spacing, spacing, spacing, spacing);
    grid->setSpacing(spacing);

    // --------------------------------------------------------

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->dateCreatedSel, SLOT(setEnabled(bool)));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            d->dateDigitalizedSel, SLOT(setEnabled(bool)));

    connect(d->dateVideoCheck, SIGNAL(toggled(bool)),
            d->dateVideoSel, SLOT(setEnabled(bool)));

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->zoneCreatedSel, SLOT(setEnabled(bool)));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            d->zoneDigitalizedSel, SLOT(setEnabled(bool)));

    connect(d->dateVideoCheck, SIGNAL(toggled(bool)),
            d->zoneVideoSel, SLOT(setEnabled(bool)));

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            d->syncEXIFDateCheck, SLOT(setEnabled(bool)));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            d->cityEdit, SLOT(setEnabled(bool)));

    connect(d->sublocationCheck, SIGNAL(toggled(bool)),
            d->sublocationEdit, SLOT(setEnabled(bool)));

    connect(d->provinceCheck, SIGNAL(toggled(bool)),
            d->provinceEdit, SLOT(setEnabled(bool)));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            d->countryCB, SLOT(setEnabled(bool)));

    // --------------------------------------------------------

    connect(d->dateCreatedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->dateVideoCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sublocationCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->provinceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->dateCreatedSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->dateVideoSel, SIGNAL(dateTimeChanged(QDateTime)),
            this, SIGNAL(signalModified()));

    connect(d->zoneCreatedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->zoneDigitalizedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->zoneVideoSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->setTodayCreatedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayCreated()));

    connect(d->setTodayDigitalizedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayDigitalized()));

    connect(d->setTodayVideoBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayVideo()));

    // --------------------------------------------------------

    connect(d->countryCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->cityEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->sublocationEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->provinceEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));
}

XMPOrigin::~XMPOrigin()
{
    delete d;
}

void XMPOrigin::slotSetTodayVideo()
{
    d->dateVideoSel->setDateTime(QDateTime::currentDateTime());
    d->zoneVideoSel->setToUTC();
}

void XMPOrigin::slotSetTodayCreated()
{
    d->dateCreatedSel->setDateTime(QDateTime::currentDateTime());
    d->zoneCreatedSel->setToUTC();
}

void XMPOrigin::slotSetTodayDigitalized()
{
    d->dateDigitalizedSel->setDateTime(QDateTime::currentDateTime());
    d->zoneDigitalizedSel->setToUTC();
}

bool XMPOrigin::syncEXIFDateIsChecked() const
{
    return d->syncEXIFDateCheck->isChecked();
}

void XMPOrigin::setCheckedSyncEXIFDate(bool c)
{
    d->syncEXIFDateCheck->setChecked(c);
}

QDateTime XMPOrigin::getXMPCreationDate() const
{
    return d->dateCreatedSel->dateTime();
}

void XMPOrigin::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString     data;
    QStringList code, list;
    QDateTime   dateTime;
    QString     dateTimeStr;

    dateTimeStr = meta.getXmpTagString("Xmp.photoshop.DateCreated", false);

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.CreateDate", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.ModifyDate", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.exif.DateTimeOriginal", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.tiff.DateTime", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.ModifyDate", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.xmp.MetadataDate", false);
    }

    d->dateCreatedSel->setDateTime(QDateTime::currentDateTime());
    d->dateCreatedCheck->setChecked(false);
    d->zoneCreatedSel->setToUTC();

    if (!dateTimeStr.isEmpty())
    {
        dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);

        if (dateTime.isValid())
        {
            d->dateCreatedSel->setDateTime(dateTime);
            d->dateCreatedCheck->setChecked(true);
            d->zoneCreatedSel->setTimeZone(dateTimeStr);
        }
    }

    d->dateCreatedSel->setEnabled(d->dateCreatedCheck->isChecked());
    d->zoneCreatedSel->setEnabled(d->dateCreatedCheck->isChecked());
    d->syncEXIFDateCheck->setEnabled(d->dateCreatedCheck->isChecked());

    dateTimeStr = meta.getXmpTagString("Xmp.exif.DateTimeDigitized", false);

    d->dateDigitalizedSel->setDateTime(QDateTime::currentDateTime());
    d->dateDigitalizedCheck->setChecked(false);
    d->zoneDigitalizedSel->setToUTC();

    if (!dateTimeStr.isEmpty())
    {
        dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);

        if (dateTime.isValid())
        {
            d->dateDigitalizedSel->setDateTime(dateTime);
            d->dateDigitalizedCheck->setChecked(true);
            d->zoneDigitalizedSel->setTimeZone(dateTimeStr);
        }
    }

    d->dateDigitalizedSel->setEnabled(d->dateDigitalizedCheck->isChecked());
    d->zoneDigitalizedSel->setEnabled(d->dateDigitalizedCheck->isChecked());

    dateTimeStr = meta.getXmpTagString("Xmp.video.DateTimeOriginal", false);

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.video.DateTimeDigitized", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.video.ModificationDate", false);
    }

    if (dateTimeStr.isEmpty())
    {
        dateTimeStr = meta.getXmpTagString("Xmp.video.DateUTC", false);
    }

    d->dateVideoSel->setDateTime(QDateTime::currentDateTime());
    d->dateVideoCheck->setChecked(false);
    d->zoneVideoSel->setToUTC();

    if (!dateTimeStr.isEmpty())
    {
        dateTime = QDateTime::fromString(dateTimeStr, Qt::ISODate);

        if (dateTime.isValid())
        {
            d->dateVideoSel->setDateTime(dateTime);
            d->dateVideoCheck->setChecked(true);
            d->zoneVideoSel->setTimeZone(dateTimeStr);
        }
    }

    d->dateVideoSel->setEnabled(d->dateVideoCheck->isChecked());
    d->zoneVideoSel->setEnabled(d->dateVideoCheck->isChecked());

    d->cityEdit->clear();
    d->cityCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.City", false);

    if (!data.isNull())
    {
        d->cityEdit->setText(data);
        d->cityCheck->setChecked(true);
    }

    d->cityEdit->setEnabled(d->cityCheck->isChecked());

    d->sublocationEdit->clear();
    d->sublocationCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.Location", false);

    if (!data.isNull())
    {
        d->sublocationEdit->setText(data);
        d->sublocationCheck->setChecked(true);
    }

    d->sublocationEdit->setEnabled(d->sublocationCheck->isChecked());

    d->provinceEdit->clear();
    d->provinceCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.photoshop.State", false);

    if (!data.isNull())
    {
        d->provinceEdit->setText(data);
        d->provinceCheck->setChecked(true);
    }

    d->provinceEdit->setEnabled(d->provinceCheck->isChecked());

    d->countryCB->setCurrentIndex(0);
    d->countryCheck->setChecked(false);
    data = meta.getXmpTagString("Xmp.iptc.CountryCode", false);

    if (!data.isNull())
    {
        int item = -1;

        for (int i = 0 ; i < d->countryCB->count() ; ++i)
        {
            if (d->countryCB->itemText(i).left(3) == data)
                item = i;
        }

        if (item != -1)
        {
            d->countryCB->setCurrentIndex(item);
            d->countryCheck->setChecked(true);
        }
        else
        {
            d->countryCheck->setValid(false);
        }
    }

    d->countryCB->setEnabled(d->countryCheck->isChecked());

    blockSignals(false);
}

void XMPOrigin::applyMetadata(const DMetadata& meta)
{
    QString xmpDateTimeFormat = QLatin1String("yyyy-MM-ddThh:mm:ss");

    if (d->dateCreatedCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.DateCreated",
                             getXMPCreationDate().toString(xmpDateTimeFormat) +
                             d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.xmp.CreateDate",
                             getXMPCreationDate().toString(xmpDateTimeFormat) +
                             d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.exif.DateTimeOriginal",
                             getXMPCreationDate().toString(xmpDateTimeFormat) +
                             d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.tiff.DateTime",
                             getXMPCreationDate().toString(xmpDateTimeFormat) +
                             d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.xmp.ModifyDate",
                             getXMPCreationDate().toString(xmpDateTimeFormat) +
                             d->zoneCreatedSel->getTimeZone());
        meta.setXmpTagString("Xmp.xmp.MetadataDate",
                             getXMPCreationDate().toString(xmpDateTimeFormat) +
                             d->zoneCreatedSel->getTimeZone());

        if (syncEXIFDateIsChecked())
        {
            meta.setExifTagString("Exif.Image.DateTime",
                                  getXMPCreationDate().toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.DateCreated");
        meta.removeXmpTag("Xmp.xmp.CreateDate");
        meta.removeXmpTag("Xmp.exif.DateTimeOriginal");
        meta.removeXmpTag("Xmp.tiff.DateTime");
        meta.removeXmpTag("Xmp.xmp.ModifyDate");
        meta.removeXmpTag("Xmp.xmp.MetadataDate");
    }

    if (d->dateDigitalizedCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.exif.DateTimeDigitized",
                             d->dateDigitalizedSel->dateTime().toString(xmpDateTimeFormat) +
                             d->zoneDigitalizedSel->getTimeZone());
    }
    else
    {
        meta.removeXmpTag("Xmp.exif.DateTimeDigitized");
    }

    if (d->dateVideoCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.video.DateTimeOriginal",
                             d->dateVideoSel->dateTime().toString(xmpDateTimeFormat) +
                             d->zoneVideoSel->getTimeZone());
        meta.setXmpTagString("Xmp.video.DateTimeDigitized",
                             d->dateVideoSel->dateTime().toString(xmpDateTimeFormat) +
                             d->zoneVideoSel->getTimeZone());
        meta.setXmpTagString("Xmp.video.ModificationDate",
                             d->dateVideoSel->dateTime().toString(xmpDateTimeFormat) +
                             d->zoneVideoSel->getTimeZone());
        meta.setXmpTagString("Xmp.video.DateUTC",
                             d->dateVideoSel->dateTime().toString(xmpDateTimeFormat) +
                             d->zoneVideoSel->getTimeZone());
    }
    else
    {
        meta.removeXmpTag("Xmp.video.DateTimeOriginal");
        meta.removeXmpTag("Xmp.video.DateTimeDigitized");
        meta.removeXmpTag("Xmp.video.ModificationDate");
        meta.removeXmpTag("Xmp.video.DateUTC");
    }

    if (d->cityCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.City", d->cityEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.City");
    }

    if (d->sublocationCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.iptc.Location", d->sublocationEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.iptc.Location");
    }

    if (d->provinceCheck->isChecked())
    {
        meta.setXmpTagString("Xmp.photoshop.State", d->provinceEdit->text());
    }
    else
    {
        meta.removeXmpTag("Xmp.photoshop.State");
    }

    if (d->countryCheck->isChecked())
    {
        QString countryName = d->countryCB->currentText().mid(6);
        QString countryCode = d->countryCB->currentText().left(3);
        meta.setXmpTagString("Xmp.iptc.CountryCode", countryCode);
        meta.setXmpTagString("Xmp.photoshop.Country", countryName);
    }
    else if (d->countryCheck->isValid())
    {
        meta.removeXmpTag("Xmp.iptc.CountryCode");
        meta.removeXmpTag("Xmp.photoshop.Country");
    }
}

} // namespace DigikamGenericMetadataEditPlugin
