/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : A date selection widget.
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1997      by Tim D. Gilman <tdgilman at best dot org>
 * SPDX-FileCopyrightText: 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * SPDX-FileCopyrightText: 2007      by John Layt <john at layt dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DDATE_PICKER_PRIVATE_H
#define DIGIKAM_DDATE_PICKER_PRIVATE_H

// Qt includes

#include <QDate>
#include <QLineEdit>
#include <QValidator>
#include <QApplication>
#include <QComboBox>
#include <QToolButton>
#include <QBoxLayout>
#include <QSize>

// Local includes

#include "ddatepicker.h"

namespace Digikam
{

class Q_DECL_HIDDEN DatePickerValidator : public QValidator
{
    Q_OBJECT

public:

    explicit DatePickerValidator(DDatePicker* const parent);

    State validate(QString& text, int&) const override;

private:

    DDatePicker* m_picker;
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN DatePickerYearSelector : public QLineEdit
{
    Q_OBJECT

public:

    explicit DatePickerYearSelector(const QDate& currentDate, QWidget* const parent = nullptr);

    int  year() const;
    void setYear(int year);

public Q_SLOTS:

    void yearEnteredSlot();

Q_SIGNALS:

    void closeMe(int);

protected:

    QIntValidator* val;
    int            result;

private:

    QDate          oldDate;

    Q_DISABLE_COPY(DatePickerYearSelector)
};

// ------------------------------------------------------------------------------

class Q_DECL_HIDDEN DDatePicker::Private
{
public:

    explicit Private(DDatePicker* const qq);

    void fillWeeksCombo();
    QDate validDateInYearMonth(int year, int month);

public:

    /// the date table
    DDatePicker*         q;

    QToolButton*         closeButton;
    QComboBox*           selectWeek;
    QToolButton*         todayButton;
    QBoxLayout*          navigationLayout;

    /// the year forward button
    QToolButton*         yearForward;

    /// the year backward button
    QToolButton*         yearBackward;

    /// the month forward button
    QToolButton*         monthForward;

    /// the month backward button
    QToolButton*         monthBackward;

    /// the button for selecting the month directly
    QToolButton*         selectMonth;

    /// the button for selecting the year directly
    QToolButton*         selectYear;

    /// the line edit to enter the date directly
    QLineEdit*           line;

    /// the validator for the line edit:
    DatePickerValidator* val;

    /// the date table
    DDateTable*          table;

    /// the widest month string in pixels:
    QSize                maxMonthRect;

    /// the font size for the widget
    int                  fontsize;
};

} // namespace Digikam

#endif // DIGIKAM_DDATE_PICKER_PRIVATE_H
