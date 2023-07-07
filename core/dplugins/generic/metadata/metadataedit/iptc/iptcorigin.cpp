/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-10-13
 * Description : IPTC origin settings page.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iptcorigin.h"

// Qt includes

#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QTimeEdit>
#include <QGridLayout>
#include <QApplication>
#include <QStyle>
#include <QComboBox>
#include <QDateEdit>
#include <QLineEdit>
#include <QToolTip>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dlayoutbox.h"
#include "squeezedcombobox.h"
#include "metadatacheckbox.h"
#include "timezonecombobox.h"
#include "multivaluesedit.h"
#include "countryselector.h"
#include "dexpanderbox.h"

namespace DigikamGenericMetadataEditPlugin
{

class Q_DECL_HIDDEN IPTCOrigin::Private
{
public:

    explicit Private()
      : dateCreatedCheck(nullptr),
        dateDigitalizedCheck(nullptr),
        timeCreatedCheck(nullptr),
        timeDigitalizedCheck(nullptr),
        syncEXIFDateCheck(nullptr),
        cityCheck(nullptr),
        sublocationCheck(nullptr),
        provinceCheck(nullptr),
        timeCreatedSel(nullptr),
        timeDigitalizedSel(nullptr),
        zoneCreatedSel(nullptr),
        zoneDigitalizedSel(nullptr),
        setTodayCreatedBtn(nullptr),
        setTodayDigitalizedBtn(nullptr),
        dateCreatedSel(nullptr),
        dateDigitalizedSel(nullptr),
        cityEdit(nullptr),
        sublocationEdit(nullptr),
        provinceEdit(nullptr),
        locationEdit(nullptr),
        countryCheck(nullptr),
        countryCB(nullptr)
    {
    }

    QCheckBox*                     dateCreatedCheck;
    QCheckBox*                     dateDigitalizedCheck;
    QCheckBox*                     timeCreatedCheck;
    QCheckBox*                     timeDigitalizedCheck;
    QCheckBox*                     syncEXIFDateCheck;
    QCheckBox*                     cityCheck;
    QCheckBox*                     sublocationCheck;
    QCheckBox*                     provinceCheck;

    QTimeEdit*                     timeCreatedSel;
    QTimeEdit*                     timeDigitalizedSel;

    TimeZoneComboBox*              zoneCreatedSel;
    TimeZoneComboBox*              zoneDigitalizedSel;

    QPushButton*                   setTodayCreatedBtn;
    QPushButton*                   setTodayDigitalizedBtn;

    QDateEdit*                     dateCreatedSel;
    QDateEdit*                     dateDigitalizedSel;

    QLineEdit*                     cityEdit;
    QLineEdit*                     sublocationEdit;
    QLineEdit*                     provinceEdit;

    MultiValuesEdit*               locationEdit;

    MetadataCheckBox*              countryCheck;

    CountrySelector*               countryCB;
};

IPTCOrigin::IPTCOrigin(QWidget* const parent)
    : MetadataEditPage(parent),
      d               (new Private)
{
    QGridLayout* const grid = new QGridLayout(widget());
    QString dateFormat      = QLocale().dateFormat(QLocale::ShortFormat);

    if (!dateFormat.contains(QLatin1String("yyyy")))
    {
        dateFormat.replace(QLatin1String("yy"),
                           QLatin1String("yyyy"));
    }

    QString timeFormat = QLatin1String("hh:mm:ss");

    // --------------------------------------------------------

    d->dateDigitalizedCheck   = new QCheckBox(i18n("Digitization date"), this);
    d->timeDigitalizedCheck   = new QCheckBox(i18n("Digitization time"), this);
    d->zoneDigitalizedSel     = new TimeZoneComboBox(this);

    d->dateDigitalizedSel     = new QDateEdit(this);
    d->dateDigitalizedSel->setDisplayFormat(dateFormat);

    d->timeDigitalizedSel     = new QTimeEdit(this);
    d->timeDigitalizedSel->setDisplayFormat(timeFormat);

    d->setTodayDigitalizedBtn = new QPushButton();
    d->setTodayDigitalizedBtn->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));
    d->setTodayDigitalizedBtn->setWhatsThis(i18n("Set digitization date to today"));

    d->dateDigitalizedSel->setWhatsThis(i18n("Set here the creation date of "
                                             "digital representation."));
    d->timeDigitalizedSel->setWhatsThis(i18n("Set here the creation time of "
                                             "digital representation."));
    d->zoneDigitalizedSel->setWhatsThis(i18n("Set here the time zone of "
                                             "digital representation."));

    slotSetTodayDigitalized();

    // --------------------------------------------------------

    d->dateCreatedCheck   = new QCheckBox(i18n("Creation date"), this);
    d->timeCreatedCheck   = new QCheckBox(i18n("Creation time"), this);
    d->zoneCreatedSel     = new TimeZoneComboBox(this);

    d->dateCreatedSel     = new QDateEdit(this);
    d->dateCreatedSel->setDisplayFormat(dateFormat);

    d->timeCreatedSel     = new QTimeEdit(this);
    d->timeCreatedSel->setDisplayFormat(timeFormat);

    d->syncEXIFDateCheck  = new QCheckBox(i18n("Sync Exif creation date"), this);

    d->setTodayCreatedBtn = new QPushButton();
    d->setTodayCreatedBtn->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));
    d->setTodayCreatedBtn->setWhatsThis(i18n("Set creation date to today"));

    d->dateCreatedSel->setWhatsThis(i18n("Set here the creation date of "
                                         "intellectual content."));
    d->timeCreatedSel->setWhatsThis(i18n("Set here the creation time of "
                                         "intellectual content."));
    d->zoneCreatedSel->setWhatsThis(i18n("Set here the time zone of "
                                         "intellectual content."));

    slotSetTodayCreated();

    // --------------------------------------------------------

    d->locationEdit = new MultiValuesEdit(this, i18n("Location:"),
                          i18n("Set here the full country name referenced by the content."));

    // --------------------------------------------------------

    d->cityCheck = new QCheckBox(i18n("City:"), this);
    d->cityEdit  = new QLineEdit(this);
    d->cityEdit->setClearButtonEnabled(true);
    d->cityEdit->setMaxLength(32);
    d->cityEdit->setPlaceholderText(i18n("Set here the city of the content"));
    d->cityEdit->setWhatsThis(i18n("Set here the city of content origin. "
                                   "This field is limited to 32 characters."));

    // --------------------------------------------------------

    d->sublocationCheck = new QCheckBox(i18n("Sublocation:"), this);
    d->sublocationEdit  = new QLineEdit(this);
    d->sublocationEdit->setClearButtonEnabled(true);
    d->sublocationEdit->setMaxLength(32);
    d->sublocationEdit->setPlaceholderText(i18n("Set here the content location"));
    d->sublocationEdit->setWhatsThis(i18n("Set here the content location within city. "
                                          "This field is limited to 32 characters."));

    // --------------------------------------------------------

    d->provinceCheck = new QCheckBox(i18n("State/Province:"), this);
    d->provinceEdit  = new QLineEdit(this);
    d->provinceEdit->setClearButtonEnabled(true);
    d->provinceEdit->setMaxLength(32);
    d->provinceEdit->setPlaceholderText(i18n("Set here the Province or State of the content"));
    d->provinceEdit->setWhatsThis(i18n("Set here the Province or State of content origin. "
                                       "This field is limited to 32 characters."));

    // --------------------------------------------------------

    d->countryCheck = new MetadataCheckBox(i18n("Country:"), this);
    d->countryCB    = new CountrySelector(this);
    d->countryCB->setWhatsThis(i18n("Select here country name of content origin."));

    // Remove 2 last items for the list (separator + Unknown item)

    d->countryCB->removeItem(d->countryCB->count()-1);
    d->countryCB->removeItem(d->countryCB->count()-1);

    QStringList list;

    for (int i = 0 ; i < d->countryCB->count() ; ++i)
    {
        list.append(d->countryCB->itemText(i));
    }

    d->locationEdit->setData(list);

    // --------------------------------------------------------

    QLabel* const note = new QLabel(i18n("<b>Note: "
                 "<a href='https://en.wikipedia.org/wiki/IPTC_Information_Interchange_Model'>IPTC</a> "
                 "text tags are limited string sizes. Use contextual help for details. "
                 "Consider to use <a href='https://en.wikipedia.org/wiki/Extensible_Metadata_Platform'>XMP</a> instead.</b>"),
                 this);
    note->setOpenExternalLinks(true);
    note->setWordWrap(true);
    note->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    // --------------------------------------------------------

    grid->addWidget(d->dateDigitalizedCheck,                0, 0, 1, 2);
    grid->addWidget(d->timeDigitalizedCheck,                0, 2, 1, 2);
    grid->addWidget(d->dateDigitalizedSel,                  1, 0, 1, 2);
    grid->addWidget(d->timeDigitalizedSel,                  1, 2, 1, 1);
    grid->addWidget(d->zoneDigitalizedSel,                  1, 3, 1, 1);
    grid->addWidget(d->setTodayDigitalizedBtn,              1, 5, 1, 1);
    grid->addWidget(d->dateCreatedCheck,                    2, 0, 1, 2);
    grid->addWidget(d->timeCreatedCheck,                    2, 2, 1, 2);
    grid->addWidget(d->dateCreatedSel,                      3, 0, 1, 2);
    grid->addWidget(d->timeCreatedSel,                      3, 2, 1, 1);
    grid->addWidget(d->zoneCreatedSel,                      3, 3, 1, 1);
    grid->addWidget(d->setTodayCreatedBtn,                  3, 5, 1, 1);
    grid->addWidget(d->syncEXIFDateCheck,                   5, 0, 1, 6);
    grid->addWidget(d->locationEdit,                        6, 0, 1, 6);
    grid->addWidget(new DLineWidget(Qt::Horizontal, this),  7, 0, 1, 6);
    grid->addWidget(d->cityCheck,                           8, 0, 1, 1);
    grid->addWidget(d->cityEdit,                            8, 1, 1, 5);
    grid->addWidget(d->sublocationCheck,                    9, 0, 1, 1);
    grid->addWidget(d->sublocationEdit,                     9, 1, 1, 5);
    grid->addWidget(d->provinceCheck,                      10, 0, 1, 1);
    grid->addWidget(d->provinceEdit,                       10, 1, 1, 5);
    grid->addWidget(d->countryCheck,                       11, 0, 1, 1);
    grid->addWidget(d->countryCB,                          11, 1, 1, 5);
    grid->addWidget(note,                                  12, 0, 1, 6);
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

    connect(d->timeCreatedCheck, SIGNAL(toggled(bool)),
            d->timeCreatedSel, SLOT(setEnabled(bool)));

    connect(d->timeDigitalizedCheck, SIGNAL(toggled(bool)),
            d->timeDigitalizedSel, SLOT(setEnabled(bool)));

    connect(d->timeCreatedCheck, SIGNAL(toggled(bool)),
            d->zoneCreatedSel, SLOT(setEnabled(bool)));

    connect(d->timeDigitalizedCheck, SIGNAL(toggled(bool)),
            d->zoneDigitalizedSel, SLOT(setEnabled(bool)));

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

    connect(d->timeCreatedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->timeDigitalizedCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->cityCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->sublocationCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->provinceCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->countryCheck, SIGNAL(toggled(bool)),
            this, SIGNAL(signalModified()));

    connect(d->locationEdit, SIGNAL(signalModified()),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->dateCreatedSel, SIGNAL(dateChanged(QDate)),
            this, SIGNAL(signalModified()));

    connect(d->dateDigitalizedSel, SIGNAL(dateChanged(QDate)),
            this, SIGNAL(signalModified()));

    connect(d->timeCreatedSel, SIGNAL(timeChanged(QTime)),
            this, SIGNAL(signalModified()));

    connect(d->timeDigitalizedSel, SIGNAL(timeChanged(QTime)),
            this, SIGNAL(signalModified()));

    connect(d->zoneCreatedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->zoneDigitalizedSel, SIGNAL(currentIndexChanged(QString)),
            this, SIGNAL(signalModified()));

    // --------------------------------------------------------

    connect(d->setTodayCreatedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayCreated()));

    connect(d->setTodayDigitalizedBtn, SIGNAL(clicked()),
            this, SLOT(slotSetTodayDigitalized()));

    // --------------------------------------------------------

    connect(d->countryCB, SIGNAL(activated(int)),
            this, SIGNAL(signalModified()));

    connect(d->cityEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->cityEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotLineEditModified()));

    connect(d->sublocationEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->sublocationEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotLineEditModified()));

    connect(d->provinceEdit, SIGNAL(textChanged(QString)),
            this, SIGNAL(signalModified()));

    connect(d->provinceEdit, SIGNAL(textChanged(QString)),
            this, SLOT(slotLineEditModified()));
}

IPTCOrigin::~IPTCOrigin()
{
    delete d;
}

void IPTCOrigin::slotSetTodayCreated()
{
    d->dateCreatedSel->setDate(QDate::currentDate());
    d->timeCreatedSel->setTime(QTime::currentTime());
    d->zoneCreatedSel->setToUTC();
}

void IPTCOrigin::slotSetTodayDigitalized()
{
    d->dateDigitalizedSel->setDate(QDate::currentDate());
    d->timeDigitalizedSel->setTime(QTime::currentTime());
    d->zoneDigitalizedSel->setToUTC();
}

bool IPTCOrigin::syncEXIFDateIsChecked() const
{
    return d->syncEXIFDateCheck->isChecked();
}

void IPTCOrigin::setCheckedSyncEXIFDate(bool c)
{
    d->syncEXIFDateCheck->setChecked(c);
}

QDateTime IPTCOrigin::getIPTCCreationDate() const
{
    return QDateTime(d->dateCreatedSel->date(), d->timeCreatedSel->time());
}

void IPTCOrigin::slotLineEditModified()
{
    QLineEdit* const ledit = dynamic_cast<QLineEdit*>(sender());

    if (!ledit)
    {
        return;
    }

    QToolTip::showText(ledit->mapToGlobal(QPoint(0, (-1)*(ledit->height() + 16))),
                       i18np("%1 character left", "%1 characters left", ledit->maxLength() - ledit->text().size()),
                       ledit);
}

void IPTCOrigin::readMetadata(const DMetadata& meta)
{
    blockSignals(true);

    QString     data;
    QStringList code, list;
    QDate       date;
    QTime       time;
    QString     dateStr, timeStr;

    dateStr = meta.getIptcTagString("Iptc.Application2.DateCreated", false);
    timeStr = meta.getIptcTagString("Iptc.Application2.TimeCreated", false);

    d->dateCreatedSel->setDate(QDate::currentDate());
    d->dateCreatedCheck->setChecked(false);

    if (!dateStr.isEmpty())
    {
        date = QDate::fromString(dateStr, Qt::ISODate);

        if (date.isValid())
        {
            d->dateCreatedSel->setDate(date);
            d->dateCreatedCheck->setChecked(true);
        }
    }

    d->dateCreatedSel->setEnabled(d->dateCreatedCheck->isChecked());
    d->syncEXIFDateCheck->setEnabled(d->dateCreatedCheck->isChecked());

    d->timeCreatedSel->setTime(QTime::currentTime());
    d->timeCreatedCheck->setChecked(false);
    d->zoneCreatedSel->setToUTC();

    if (!timeStr.isEmpty())
    {
        time = QTime::fromString(timeStr, Qt::ISODate);

        if (time.isValid())
        {
            d->timeCreatedSel->setTime(time);
            d->timeCreatedCheck->setChecked(true);
            d->zoneCreatedSel->setTimeZone(timeStr);
        }
    }

    d->timeCreatedSel->setEnabled(d->timeCreatedCheck->isChecked());
    d->zoneCreatedSel->setEnabled(d->timeCreatedCheck->isChecked());

    dateStr = meta.getIptcTagString("Iptc.Application2.DigitizationDate", false);
    timeStr = meta.getIptcTagString("Iptc.Application2.DigitizationTime", false);

    d->dateDigitalizedSel->setDate(QDate::currentDate());
    d->dateDigitalizedCheck->setChecked(false);

    if (!dateStr.isEmpty())
    {
        date = QDate::fromString(dateStr, Qt::ISODate);

        if (date.isValid())
        {
            d->dateDigitalizedSel->setDate(date);
            d->dateDigitalizedCheck->setChecked(true);
        }
    }

    d->dateDigitalizedSel->setEnabled(d->dateDigitalizedCheck->isChecked());

    d->timeDigitalizedSel->setTime(QTime::currentTime());
    d->timeDigitalizedCheck->setChecked(false);
    d->zoneDigitalizedSel->setToUTC();

    if (!timeStr.isEmpty())
    {
        time = QTime::fromString(timeStr, Qt::ISODate);

        if (time.isValid())
        {
            d->timeDigitalizedSel->setTime(time);
            d->timeDigitalizedCheck->setChecked(true);
            d->zoneDigitalizedSel->setTimeZone(timeStr);
        }
    }

    d->timeDigitalizedSel->setEnabled(d->timeDigitalizedCheck->isChecked());
    d->zoneDigitalizedSel->setEnabled(d->timeDigitalizedCheck->isChecked());

    code = meta.getIptcTagsStringList("Iptc.Application2.LocationCode", false);

    for (QStringList::Iterator it = code.begin(); it != code.end(); ++it)
    {
        QStringList lst = d->locationEdit->getData();
        QStringList::Iterator it2;

        for (it2 = lst.begin() ; it2 != lst.end() ; ++it2)
        {
            if ((*it2).left(3) == (*it))
            {
                list.append(*it2);
                break;
            }
        }

        if (it2 == lst.end())
        {
            d->locationEdit->setValid(false);
        }
    }

    d->locationEdit->setValues(list);

    d->cityEdit->clear();
    d->cityCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.City", false);

    if (!data.isNull())
    {
        d->cityEdit->setText(data);
        d->cityCheck->setChecked(true);
    }

    d->cityEdit->setEnabled(d->cityCheck->isChecked());

    d->sublocationEdit->clear();
    d->sublocationCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.SubLocation", false);

    if (!data.isNull())
    {
        d->sublocationEdit->setText(data);
        d->sublocationCheck->setChecked(true);
    }

    d->sublocationEdit->setEnabled(d->sublocationCheck->isChecked());

    d->provinceEdit->clear();
    d->provinceCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.ProvinceState", false);

    if (!data.isNull())
    {
        d->provinceEdit->setText(data);
        d->provinceCheck->setChecked(true);
    }

    d->provinceEdit->setEnabled(d->provinceCheck->isChecked());

    d->countryCB->setCurrentIndex(0);
    d->countryCheck->setChecked(false);
    data = meta.getIptcTagString("Iptc.Application2.CountryCode", false);

    if (!data.isNull())
    {
        int item = -1;

        for (int i = 0 ; i < d->countryCB->count() ; ++i)
        {
            if (d->countryCB->itemText(i).left(3) == data)
            {
                item = i;
            }
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

void IPTCOrigin::applyMetadata(const DMetadata& meta)
{
    if (d->dateCreatedCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.DateCreated", getIPTCCreationDate().toString(Qt::ISODate));

        if (syncEXIFDateIsChecked())
        {
            meta.setExifTagString("Exif.Image.DateTime",
                                  getIPTCCreationDate().toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
        }
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.DateCreated");
    }

    if (d->dateDigitalizedCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.DigitizationDate", d->dateDigitalizedSel->date().toString(Qt::ISODate));
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.DigitizationDate");
    }

    if (d->timeCreatedCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.TimeCreated", d->timeCreatedSel->time().toString(Qt::ISODate) +
                                                               d->zoneCreatedSel->getTimeZone());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.TimeCreated");
    }

    if (d->timeDigitalizedCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.DigitizationTime", d->timeDigitalizedSel->time().toString(Qt::ISODate) +
                                                                    d->zoneDigitalizedSel->getTimeZone());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.DigitizationTime");
    }

    QStringList oldList, newList;

    if (d->locationEdit->getValues(oldList, newList))
    {
        QStringList oldCode, newCode, oldName, newName;

        for (QStringList::Iterator it = oldList.begin() ; it != oldList.end() ; ++it)
        {
            oldCode.append((*it).left(3));
            oldName.append((*it).mid(6));
        }

        for (QStringList::Iterator it2 = newList.begin() ; it2 != newList.end() ; ++it2)
        {
            newCode.append((*it2).left(3));
            newName.append((*it2).mid(6));
        }

        meta.setIptcTagsStringList("Iptc.Application2.LocationCode", 3, oldCode, newCode);
        meta.setIptcTagsStringList("Iptc.Application2.LocationName", 64, oldName, newName);
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.LocationCode");
        meta.removeIptcTag("Iptc.Application2.LocationName");
    }

    if (d->cityCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.City", d->cityEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.City");
    }

    if (d->sublocationCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.SubLocation", d->sublocationEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.SubLocation");
    }

    if (d->provinceCheck->isChecked())
    {
        meta.setIptcTagString("Iptc.Application2.ProvinceState", d->provinceEdit->text());
    }
    else
    {
        meta.removeIptcTag("Iptc.Application2.ProvinceState");
    }

    if      (d->countryCheck->isChecked())
    {
        QString countryName = d->countryCB->currentText().mid(6);
        QString countryCode = d->countryCB->currentText().left(3);
        meta.setIptcTagString("Iptc.Application2.CountryCode", countryCode);
        meta.setIptcTagString("Iptc.Application2.CountryName", countryName);
    }
    else if (d->countryCheck->isValid())
    {
        meta.removeIptcTag("Iptc.Application2.CountryCode");
        meta.removeIptcTag("Iptc.Application2.CountryName");
    }
}

} // namespace DigikamGenericMetadataEditPlugin
