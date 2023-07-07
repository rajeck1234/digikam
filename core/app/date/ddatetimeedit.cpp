/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a widget to edit time stamp.
 *
 * SPDX-FileCopyrightText: 2005      by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "ddatetimeedit.h"

// Qt includes

#include <QDateTimeEdit>

// Local includes

#include "ddateedit.h"

namespace Digikam
{

class Q_DECL_HIDDEN DDateTimeEdit::Private
{
public:

    explicit Private()
      : timePopUp(nullptr),
        datePopUp(nullptr)
    {
    }

    QTimeEdit* timePopUp;
    DDateEdit* datePopUp;
};

DDateTimeEdit::DDateTimeEdit(QWidget* const parent, const QString& name)
    : DHBox(parent),
      d    (new Private)
{
    setObjectName(name);

    d->datePopUp = new DDateEdit(this, QLatin1String("datepopup"));
    d->timePopUp = new QTimeEdit(QTime::currentTime(), this);

    d->timePopUp->setDisplayFormat(QLatin1String("hh:mm:ss"));

    connect(d->datePopUp, &DDateEdit::dateChanged,
            this, &DDateTimeEdit::slotDateTimeChanged);

    connect(d->timePopUp, &QTimeEdit::timeChanged,
            this, &DDateTimeEdit::slotDateTimeChanged);
}

DDateTimeEdit::~DDateTimeEdit()
{
    delete d->datePopUp;
    d->datePopUp = nullptr;

    delete d->timePopUp;
    d->timePopUp = nullptr;

    delete d;
}

QDateTime DDateTimeEdit::dateTime() const
{
    return QDateTime(d->datePopUp->date(), d->timePopUp->time());
}

void DDateTimeEdit::setDateTime(const QDateTime& dateTime)
{
    d->datePopUp->setDate(dateTime.date());
    d->timePopUp->setTime(dateTime.time());
}

void DDateTimeEdit::slotDateTimeChanged()
{
    Q_EMIT dateTimeChanged(dateTime());
}

} // namespace Digikam
