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

#include "ddatepicker_p.h"

// Qt includes

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QLayout>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QStyle>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "ddatetable_p.h"
#include "dpopupframe.h"

namespace Digikam
{

DDatePicker::DDatePicker(QWidget* const parent)
    : QFrame(parent),
      d     (new Private(this))
{
    initWidget(QDate::currentDate());
}

DDatePicker::DDatePicker(const QDate& dt, QWidget* const parent)
    : QFrame(parent),
      d     (new Private(this))
{
    initWidget(dt);
}

void DDatePicker::initWidget(const QDate& dt)
{
    const int spacingHint = qMin(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                                 style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

    QBoxLayout* const topLayout = new QVBoxLayout(this);
    topLayout->setSpacing(0);
    topLayout->setContentsMargins(QMargins());

    d->navigationLayout = new QHBoxLayout();
    d->navigationLayout->setSpacing(0);
    d->navigationLayout->setContentsMargins(QMargins());
    topLayout->addLayout(d->navigationLayout);
    d->navigationLayout->addStretch();
    d->yearBackward     = new QToolButton(this);
    d->yearBackward->setAutoRaise(true);
    d->navigationLayout->addWidget(d->yearBackward);
    d->monthBackward    = new QToolButton(this);
    d->monthBackward ->setAutoRaise(true);
    d->navigationLayout->addWidget(d->monthBackward);
    d->navigationLayout->addSpacing(spacingHint);

    d->selectMonth      = new QToolButton(this);
    d->selectMonth ->setAutoRaise(true);
    d->navigationLayout->addWidget(d->selectMonth);
    d->selectYear       = new QToolButton(this);
    d->selectYear->setCheckable(true);
    d->selectYear->setAutoRaise(true);
    d->navigationLayout->addWidget(d->selectYear);
    d->navigationLayout->addSpacing(spacingHint);

    d->monthForward     = new QToolButton(this);
    d->monthForward ->setAutoRaise(true);
    d->navigationLayout->addWidget(d->monthForward);
    d->yearForward      = new QToolButton(this);
    d->yearForward ->setAutoRaise(true);
    d->navigationLayout->addWidget(d->yearForward);
    d->navigationLayout->addStretch();

    d->line             = new QLineEdit(this);
    d->val              = new DatePickerValidator(this);
    d->table            = new DDateTable(this);
    setFocusProxy(d->table);

    d->fontsize         = QFontDatabase::systemFont(QFontDatabase::GeneralFont).pointSize();

    if (d->fontsize == -1)
    {
        d->fontsize = QFontInfo(QFontDatabase::systemFont(QFontDatabase::GeneralFont)).pointSize();
    }

    d->fontsize++;                              // Make a little bigger

    d->selectWeek  = new QComboBox(this);       // read only week selection
    d->selectWeek->setFocusPolicy(Qt::NoFocus);
    d->todayButton = new QToolButton(this);
    d->todayButton->setIcon(QIcon::fromTheme(QLatin1String("view-calendar")));

    d->yearForward->setToolTip(i18n("Next year"));
    d->yearBackward->setToolTip(i18n("Previous year"));
    d->monthForward->setToolTip(i18n("Next month"));
    d->monthBackward->setToolTip(i18n("Previous month"));
    d->selectWeek->setToolTip(i18n("Select a week"));
    d->selectMonth->setToolTip(i18n("Select a month"));
    d->selectYear->setToolTip(i18n("Select a year"));
    d->todayButton->setToolTip(i18n("Select the current day"));

    // -----

    setFontSize(d->fontsize);
    d->line->setValidator(d->val);
    d->line->installEventFilter(this);

    if (QApplication::isRightToLeft())
    {
        d->yearForward->setIcon(QIcon::fromTheme(QLatin1String("arrow-left-double")));
        d->yearBackward->setIcon(QIcon::fromTheme(QLatin1String("arrow-right-double")));
        d->monthForward->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
        d->monthBackward->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
    }
    else
    {
        d->yearForward->setIcon(QIcon::fromTheme(QLatin1String("arrow-right-double")));
        d->yearBackward->setIcon(QIcon::fromTheme(QLatin1String("arrow-left-double")));
        d->monthForward->setIcon(QIcon::fromTheme(QLatin1String("go-next")));
        d->monthBackward->setIcon(QIcon::fromTheme(QLatin1String("go-previous")));
    }

    connect(d->table, SIGNAL(dateChanged(QDate)),
            this, SLOT(dateChangedSlot(QDate)));

    connect(d->table, &DDateTable::tableClicked,
            this, &DDatePicker::tableClickedSlot);

    connect(d->monthForward, &QAbstractButton::clicked,
            this, &DDatePicker::monthForwardClicked);

    connect(d->monthBackward, &QAbstractButton::clicked,
            this, &DDatePicker::monthBackwardClicked);

    connect(d->yearForward, &QAbstractButton::clicked,
            this, &DDatePicker::yearForwardClicked);

    connect(d->yearBackward, &QAbstractButton::clicked,
            this, &DDatePicker::yearBackwardClicked);

    connect(d->selectWeek, SIGNAL(activated(int)),
            this, SLOT(weekSelected(int)));

    connect(d->todayButton, &QAbstractButton::clicked,
            this, &DDatePicker::todayButtonClicked);

    connect(d->selectMonth, &QAbstractButton::clicked,
            this, &DDatePicker::selectMonthClicked);

    connect(d->selectYear, &QAbstractButton::toggled,
            this, &DDatePicker::selectYearClicked);

    connect(d->line, &QLineEdit::returnPressed,
            this, &DDatePicker::lineEnterPressed);

    topLayout->addWidget(d->table);

    QBoxLayout* const bottomLayout = new QHBoxLayout();
    bottomLayout->setContentsMargins(QMargins());
    bottomLayout->setSpacing(0);
    topLayout->addLayout(bottomLayout);

    bottomLayout->addWidget(d->todayButton);
    bottomLayout->addWidget(d->line);
    bottomLayout->addWidget(d->selectWeek);

    d->table->setDate(dt);
    dateChangedSlot(dt);    // needed because table emits changed only when newDate != oldDate
}

DDatePicker::~DDatePicker()
{
    delete d;
}

bool DDatePicker::eventFilter(QObject* o, QEvent* e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent* const k = (QKeyEvent*)e;

        if (
            (k->key() == Qt::Key_PageUp)   ||
            (k->key() == Qt::Key_PageDown) ||
            (k->key() == Qt::Key_Up)       ||
            (k->key() == Qt::Key_Down)
           )
        {
            QApplication::sendEvent(d->table, e);
            d->table->setFocus();

            return true; // eat event
        }
    }

    return QFrame::eventFilter(o, e);
}

void DDatePicker::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);
}

void DDatePicker::dateChangedSlot(const QDate& dt)
{
    QString dateFormat = locale().dateFormat(QLocale::ShortFormat);

    if (!dateFormat.contains(QLatin1String("yyyy")))
    {
        dateFormat.replace(QLatin1String("yy"),
                           QLatin1String("yyyy"));
    }

    d->line->setText(dt.toString(dateFormat));
    d->selectMonth->setText(locale().standaloneMonthName(dt.month(), QLocale::LongFormat));
    d->fillWeeksCombo();

    // calculate the item num in the week combo box; normalize selected day so as if 1.1. is the first day of the week

    QDate firstDay(dt.year(), 1, 1);

    // If we cannot successfully create the 1st of the year, this can only mean that
    // the 1st is before the earliest valid date in the current calendar system, so use
    // the earliestValidDate as the first day.
    // In particular covers the case of Gregorian where 1/1/-4713 is not a valid QDate

    d->selectWeek->setCurrentIndex((dt.dayOfYear() + firstDay.dayOfWeek() - 2) / 7);
    d->selectYear->setText(QString::number(dt.year()).rightJustified(4, QLatin1Char('0')));

    Q_EMIT dateChanged(dt);
}

void DDatePicker::tableClickedSlot()
{
    Q_EMIT dateSelected(date());
    Q_EMIT tableClicked();
}

const QDate &DDatePicker::date() const
{
    return d->table->date();
}

bool DDatePicker::setDate(const QDate& dt)
{
    // the table setDate does validity checking for us
    // this also emits dateChanged() which then calls our dateChangedSlot()

    return d->table->setDate(dt);
}

void DDatePicker::monthForwardClicked()
{
    if (!setDate(date().addMonths(1)))
    {
        QApplication::beep();
    }

    d->table->setFocus();
}

void DDatePicker::monthBackwardClicked()
{
    if (!setDate(date().addMonths(-1)))
    {
        QApplication::beep();
    }

    d->table->setFocus();
}

void DDatePicker::yearForwardClicked()
{
    if (!setDate(d->table->date().addYears(1)))
    {
        QApplication::beep();
    }

    d->table->setFocus();
}

void DDatePicker::yearBackwardClicked()
{
    if (!setDate(d->table->date().addYears(-1)))
    {
        QApplication::beep();
    }

    d->table->setFocus();
}

void DDatePicker::weekSelected(int index)
{
    QDate targetDay = d->selectWeek->itemData(index).toDateTime().date();

    if (!setDate(targetDay))
    {
        QApplication::beep();
    }

    d->table->setFocus();
}

void DDatePicker::selectMonthClicked()
{
    QDate thisDate(date());
    d->table->setFocus();

    QMenu popup(d->selectMonth);

    // Populate the pick list with all the month names, this may change by year
    // Do we need to do something here for months that fall outside valid range?

    const int monthsInYear = QDate(thisDate.year() + 1, 1, 1).addDays(-1).month();

    for (int m = 1 ; m <= monthsInYear ; ++m)
    {
        popup.addAction(locale().standaloneMonthName(m))->setData(m);
    }

    QAction* item = popup.actions().value(thisDate.month() - 1);

    // if this happens the above should already given an assertion

    if (item)
    {
        popup.setActiveAction(item);
    }

    // cancelled

    if ((item = popup.exec(d->selectMonth->mapToGlobal(QPoint(0, 0)), item)) == nullptr)
    {
        return;
    }

    // We need to create a valid date in the month selected so we can find out how many days are
    // in the month.

    QDate newDate(thisDate.year(), item->data().toInt(), 1);

    // If we have succeeded in creating a date in the new month, then try to create the new date,
    // checking we don't set a day after the last day of the month

    newDate.setDate(newDate.year(), newDate.month(), qMin(thisDate.day(), newDate.daysInMonth()));

    // Set the date, if it's invalid in any way then alert user and don't update

    if (!setDate(newDate))
    {
        QApplication::beep();
    }
}

void DDatePicker::selectYearClicked()
{
    if (!d->selectYear->isChecked())
    {
        return;
    }

    QDate thisDate(date());

    DPopupFrame* const popup             = new DPopupFrame(this);
    DatePickerYearSelector* const picker = new DatePickerYearSelector(date(), popup);
    picker->resize(picker->sizeHint());
    picker->setYear(thisDate.year());
    picker->selectAll();
    popup->setMainWidget(picker);

    connect(picker, SIGNAL(closeMe(int)),
            popup, SLOT(close(int)));

    picker->setFocus();

    if (popup->exec(d->selectYear->mapToGlobal(QPoint(0, d->selectMonth->height()))))
    {
        // We need to create a valid date in the year/month selected so we can find out how many
        // days are in the month.

        QDate newDate(picker->year(), thisDate.month(), 1);

        // If we have succeeded in creating a date in the new month, then try to create the new
        // date, checking we don't set a day after the last day of the month

        newDate = QDate(newDate.year(), newDate.month(), qMin(thisDate.day(), newDate.daysInMonth()));

        // Set the date, if it's invalid in any way then alert user and don't update

        if (!setDate(newDate))
        {
            QApplication::beep();
        }
    }

    delete popup;

    d->selectYear->setChecked(false);
}

void DDatePicker::uncheckYearSelector()
{
    d->selectYear->setChecked(false);
    d->selectYear->update();
}

void DDatePicker::changeEvent(QEvent* e)
{
    if (e && (e->type() == QEvent::EnabledChange))
    {
        if (isEnabled())
        {
            d->table->setFocus();
        }
    }
}

DDateTable *DDatePicker::dateTable() const
{
    return d->table;
}

void DDatePicker::lineEnterPressed()
{
    QString dateFormat = locale().dateFormat(QLocale::ShortFormat);

    if (!dateFormat.contains(QLatin1String("yyyy")))
    {
        dateFormat.replace(QLatin1String("yy"),
                           QLatin1String("yyyy"));
    }

    QDate newDate = QDate::fromString(d->line->text(), dateFormat);

    if (newDate.isValid())
    {
        Q_EMIT dateEntered(newDate);
        setDate(newDate);
        d->table->setFocus();
    }
    else
    {
        QApplication::beep();
    }
}

void DDatePicker::todayButtonClicked()
{
    setDate(QDate::currentDate());
    d->table->setFocus();
}

QSize DDatePicker::sizeHint() const
{
    return QWidget::sizeHint();
}

void DDatePicker::setFontSize(int s)
{
    QWidget* const buttons[] =
    {
        d->selectMonth,
        d->selectYear
    };

    const int NoOfButtons = sizeof(buttons) / sizeof(buttons[0]);
    int count;
    QFont font;
    QRect r;

    // -----

    d->fontsize = s;

    for (count = 0 ; count < NoOfButtons ; ++count)
    {
        font = buttons[count]->font();
        font.setPointSize(s);
        buttons[count]->setFont(font);
    }

    d->table->setFontSize(s);

    QFontMetrics metrics(d->selectMonth->fontMetrics());
    QString longestMonth;

    for (int i = 1 ; ; ++i)
    {
        QString str = locale().standaloneMonthName(i, QLocale::LongFormat);

        if (str.isNull())
        {
            break;
        }

        r = metrics.boundingRect(str);

        if (r.width() > d->maxMonthRect.width())
        {
            d->maxMonthRect.setWidth(r.width());
            longestMonth = str;
        }

        if (r.height() > d->maxMonthRect.height())
        {
            d->maxMonthRect.setHeight(r.height());
        }
    }

    QStyleOptionToolButton opt;
    opt.initFrom(d->selectMonth);
    opt.text       = longestMonth;

    // stolen from QToolButton

    QSize textSize = metrics.size(Qt::TextShowMnemonic, longestMonth);
    textSize.setWidth(textSize.width() + metrics.horizontalAdvance(QLatin1Char(' ')) * 2);

    int w          = textSize.width();
    int h          = textSize.height();
    opt.rect.setHeight(h);                  // PM_MenuButtonIndicator depends on the height

    QSize metricBound = style()->sizeFromContents(QStyle::CT_ToolButton, &opt, QSize(w, h), d->selectMonth);

    d->selectMonth->setMinimumSize(metricBound);
}

int DDatePicker::fontSize() const
{
    return d->fontsize;
}

void DDatePicker::setCloseButton(bool enable)
{
    if (enable == (d->closeButton != nullptr))
    {
        return;
    }

    if (enable)
    {
        d->closeButton        = new QToolButton(this);
        d->closeButton->setAutoRaise(true);

        const int spacingHint = qMin(style()->pixelMetric(QStyle::PM_LayoutHorizontalSpacing),
                                     style()->pixelMetric(QStyle::PM_LayoutVerticalSpacing));

        d->navigationLayout->addSpacing(spacingHint);
        d->navigationLayout->addWidget(d->closeButton);
        d->closeButton->setToolTip(i18nc("@action:button", "Close"));
        d->closeButton->setIcon(QIcon::fromTheme(QLatin1String("window-close")));

        connect(d->closeButton, &QAbstractButton::clicked,
                topLevelWidget(), &QWidget::close);
    }
    else
    {
        delete d->closeButton;
        d->closeButton = nullptr;
    }

    updateGeometry();
}

bool DDatePicker::hasCloseButton() const
{
    return (d->closeButton);
}

} // namespace Digikam
