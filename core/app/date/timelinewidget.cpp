/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a widget to display date and time statistics of pictures
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timelinewidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QScreen>
#include <QWindow>
#include <QLocale>
#include <QPixmap>
#include <QTimer>
#include <QDate>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "thememanager.h"
#include "digikam_globals.h"

namespace Digikam
{

class Q_DECL_HIDDEN TimeLineWidget::Private
{

public :

    typedef QPair<int, int>                           YearRefPair; ///< Year + a reference association (Month or week or day)
    typedef QPair<int, TimeLineWidget::SelectionMode> StatPair;    ///< Statistic value + selection status.

public:

    explicit Private()
      : validMouseEvent     (false),
        selMouseEvent       (false),
        maxCountByDay       (1),
        maxCountByWeek      (1),
        maxCountByMonth     (1),
        maxCountByYear      (1),
        topMargin           (3),
        bottomMargin        (20),
        barWidth            (20),
        nbItems             (10),
        startPos            (96),
        slotNextTimer       (nullptr),
        slotPreviousTimer   (nullptr),
        timeUnit            (TimeLineWidget::Month),
        scaleMode           (TimeLineWidget::LinScale)
    {
    }

    bool                         validMouseEvent;   ///< Current mouse enter event is valid to set cursor position or selection.
    bool                         selMouseEvent;     ///< Current mouse enter event is about to make a selection.

    int                          maxCountByDay;
    int                          maxCountByWeek;
    int                          maxCountByMonth;
    int                          maxCountByYear;
    int                          topMargin;
    int                          bottomMargin;
    int                          barWidth;
    int                          nbItems;
    int                          startPos;

    QDateTime                    refDateTime;       ///< Reference date-time used to draw histogram from middle of widget.
    QDateTime                    cursorDateTime;    ///< Current date-time used to draw focus cursor.
    QDateTime                    minDateTime;       ///< Higher date on histogram.
    QDateTime                    maxDateTime;       ///< Lower date on histogram.
    QDateTime                    selStartDateTime;
    QDateTime                    selMinDateTime;    ///< Lower date available on histogram.
    QDateTime                    selMaxDateTime;    ///< Higher date available on histogram.

    QTimer*                      slotNextTimer;
    QTimer*                      slotPreviousTimer;

    QPixmap                      pixmap;            ///< Used for widget double buffering.

    QRect                        focusRect;

    QMap<YearRefPair, StatPair>  dayStatMap;        ///< Store Days  count statistics.
    QMap<YearRefPair, StatPair>  weekStatMap;       ///< Store Weeks count statistics.
    QMap<YearRefPair, StatPair>  monthStatMap;      ///< Store Month count statistics.
    QMap<int,         StatPair>  yearStatMap;       ///< Store Years count statistics.

    TimeLineWidget::TimeUnit     timeUnit;
    TimeLineWidget::ScaleMode    scaleMode;
};

TimeLineWidget::TimeLineWidget(QWidget* const parent)
    : QWidget(parent),
      d      (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);
    setMinimumWidth(256);
    setMinimumHeight(192);

    QDateTime ref = QDateTime::currentDateTime();
    setCursorDateTime(ref);
    setRefDateTime(ref);

    d->slotNextTimer     = new QTimer(this);
    d->slotPreviousTimer = new QTimer(this);

    d->slotNextTimer->setInterval(10);
    d->slotPreviousTimer->setInterval(10);

    connect(d->slotNextTimer, &QTimer::timeout,
            this, &TimeLineWidget::slotNext);

    connect(d->slotPreviousTimer, &QTimer::timeout,
            this, &TimeLineWidget::slotPrevious);

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

TimeLineWidget::~TimeLineWidget()
{
    delete d;
}

void TimeLineWidget::setTimeUnit(TimeUnit timeUnit)
{
    d->timeUnit = timeUnit;
    setCursorDateTime(cursorDateTime());
    setRefDateTime(cursorDateTime());
}

TimeLineWidget::TimeUnit TimeLineWidget::timeUnit() const
{
    return d->timeUnit;
}

void TimeLineWidget::setScaleMode(ScaleMode scaleMode)
{
    d->scaleMode = scaleMode;
    update();
}

TimeLineWidget::ScaleMode TimeLineWidget::scaleMode() const
{
    return d->scaleMode;
}

int TimeLineWidget::totalIndex() const
{
    if (d->minDateTime.isNull() || d->maxDateTime.isNull())
    {
        return 0;
    }

    int        i = 0;
    QDateTime dt = d->minDateTime;

    do
    {
        dt = nextDateTime(dt);
        ++i;
    }
    while (dt < d->maxDateTime);

    return i;
}

int TimeLineWidget::indexForDateTime(const QDateTime& date) const
{
    if (d->minDateTime.isNull() || d->maxDateTime.isNull() || date.isNull())
    {
        return 0;
    }

    int        i = 0;
    QDateTime dt = d->minDateTime;

    do
    {
        dt = nextDateTime(dt);
        ++i;
    }
    while (dt < date);

    return i;
}

int TimeLineWidget::indexForRefDateTime() const
{
    return (indexForDateTime(d->refDateTime));
}

int TimeLineWidget::indexForCursorDateTime() const
{
    return (indexForDateTime(d->cursorDateTime));
}

void TimeLineWidget::setCurrentIndex(int index)
{
    if (d->minDateTime.isNull() || d->maxDateTime.isNull())
    {
        return;
    }

    int        i = 0;
    QDateTime dt = d->minDateTime;

    do
    {
        dt = nextDateTime(dt);
        ++i;
    }
    while (i <= index);

    setRefDateTime(dt);
}

void TimeLineWidget::setCursorDateTime(const QDateTime& dateTime)
{
    QDateTime dt = dateTime;
    QDate date   = dt.date();
    dt.setTime(QTime(0, 0, 0, 0));

    switch (d->timeUnit)
    {
        case Week:
        {
            // Go to the first day of week.

            int weekYear = date.year(); // Changed for week shared between 2 years (Dec/Jan).

            int weekNb   = date.weekNumber(&weekYear);

            dt           = firstDayOfWeek(weekYear, weekNb);
            break;
        }

        case Month:
        {
            // Go to the first day of month.

            dt.setDate(QDate(date.year(), date.month(), 1));
            break;
        }

        case Year:
        {
            // Go to the first day of year.

            dt.setDate(QDate(date.year(), 1, 1));
            break;
        }

        default:
        {
            break;
        }
    }

    if (d->cursorDateTime == dt)
    {
        return;
    }

    d->cursorDateTime = dt;

    Q_EMIT signalCursorPositionChanged();
}

QDateTime TimeLineWidget::cursorDateTime() const
{
    return d->cursorDateTime;
}

int TimeLineWidget::cursorInfo(QString& infoDate) const
{
    SelectionMode selected;
    QDateTime dt = cursorDateTime();
    QDate date   = dt.date();

    switch (d->timeUnit)
    {
        case Day:
        {
            infoDate = QLocale().toString(date, QLocale::LongFormat);
            break;
        }

        case Week:
        {
            int weekNb    = date.weekNumber();
            QDate endDate = date.addDays(7);
            infoDate      = i18nc("@info: week #weeknumber - month name - year string\nStart:\tEnd: ",
                                  "Week #%1 - %2 %3\n%4\t%5",
                                  weekNb,
                                  QLocale().standaloneMonthName(date.month(), QLocale::LongFormat),
                                  QLocale().toString(date, QLatin1String("yyyy")),
                                  i18nc("@info: week properties", "Start: ") + QLocale().toString(date, QLatin1String("dd")),
                                  i18nc("@info: week properties", "End: ")   + QLocale().toString(endDate, QLatin1String("dd")));
            break;
        }

        case Month:
        {
            infoDate = i18nc("@info: month-name year-string",
                             "%1 %2",
                             QLocale().standaloneMonthName(date.month(), QLocale::LongFormat),
                             QLocale().toString(date, QLatin1String("yyyy")));
            break;
        }

        case Year:
        {
            infoDate = QLocale().toString(date, QLatin1String("yyyy"));
            break;
        }
    }

    return statForDateTime(dt, selected);
}

void TimeLineWidget::setRefDateTime(const QDateTime& dateTime)
{
    QDateTime dt = dateTime;
    QDate date   = dt.date();
    dt.setTime(QTime(0, 0, 0, 0));

    switch (d->timeUnit)
    {
        case Week:
        {
            // Go to the first day of week.

            int dayWeekOffset = (-1) * (date.dayOfWeek() - 1);
            dt = dt.addDays(dayWeekOffset);
            break;
        }

        case Month:
        {
            // Go to the first day of month.

            dt.setDate(QDate(date.year(), date.month(), 1));
            break;
        }

        case Year:
        {
            // Go to the first day of year.

            dt.setDate(QDate(date.year(), 1, 1));
            break;
        }

        default:
        {
            break;
        }
    }

    d->refDateTime = dt;
    update();

    Q_EMIT signalRefDateTimeChanged();
}

void TimeLineWidget::slotResetSelection()
{
    resetSelection();
    update();
}

void TimeLineWidget::resetSelection()
{
    QMap<Private::YearRefPair, Private::StatPair>::iterator it;

    for (it = d->dayStatMap.begin() ; it != d->dayStatMap.end() ; ++it)
    {
        it.value().second = Unselected;
    }

    for (it = d->weekStatMap.begin() ; it != d->weekStatMap.end() ; ++it)
    {
        it.value().second = Unselected;
    }

    for (it = d->monthStatMap.begin() ; it != d->monthStatMap.end() ; ++it)
    {
        it.value().second = Unselected;
    }

    QMap<int, Private::StatPair>::iterator it2;

    for (it2 = d->yearStatMap.begin() ; it2 != d->yearStatMap.end() ; ++it2)
    {
        it2.value().second = Unselected;
    }
}

void TimeLineWidget::setSelectedDateRange(const DateRangeList& list)
{
    if (list.isEmpty())
    {
        return;
    }

    resetSelection();

    QDateTime start, end, dt;
    DateRangeList::const_iterator it;

    for (it = list.begin() ; it != list.end() ; ++it)
    {
        start = (*it).first;
        end   = (*it).second;

        if (end > start)
        {
            dt = start;

            do
            {
                setDateTimeSelected(dt, Selected);
                dt = dt.addDays(1);
            }
            while (dt < end);
        }
    }

    update();
}

DateRangeList TimeLineWidget::selectedDateRange(int& totalCount) const
{
    // We will parse all selections done on Days stats map.

    DateRangeList list;
    totalCount = 0;
    QMap<Private::YearRefPair, Private::StatPair>::iterator it3;
    QDateTime sdt, edt;
    QDate     date;

    if (d->selMinDateTime.isValid() &&
        d->selMaxDateTime.isValid() &&
        (d->selMinDateTime != d->selMaxDateTime))
    {
        for (it3 = d->dayStatMap.begin() ; it3 != d->dayStatMap.end() ; ++it3)
        {
            if (it3.value().second == Selected)
            {
                totalCount += it3.value().first;
            }
        }

        sdt = d->selMinDateTime;
        edt = nextDateTime(d->selMaxDateTime);
        list.append(DateRange(sdt, edt));
    }
    else
    {
        for (it3 = d->dayStatMap.begin() ; it3 != d->dayStatMap.end() ; ++it3)
        {
            if (it3.value().second == Selected)
            {
                date = QDate(it3.key().first, 1, 1);
                date = date.addDays(it3.key().second - 1);
                sdt  = startOfDay(date);
                edt  = sdt.addDays(1);
                list.append(DateRange(sdt, edt));
                totalCount += it3.value().first;
            }
        }
    }

    DateRangeList::const_iterator it, it2;

/*
        for (it = list.begin() ; it != list.end(); ++it)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << (*it).first.date().toString(Qt::ISODate) << " :: "
                                         << (*it).second.date().toString(Qt::ISODate);
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Total Count of Items = " << totalCount;
*/

    // Group contiguous date ranges to optimize query on database.

    DateRangeList list2;
    QDateTime     first, second, first2, second2;

    for (it = list.constBegin() ; it != list.constEnd() ; ++it)
    {
        first  = (*it).first;
        second = (*it).second;
        it2    = it;

        do
        {
            ++it2;

            if (it2 != list.constEnd())
            {
                first2  = (*it2).first;
                second2 = (*it2).second;

                if (first2 == second)
                {
                    second = second2;
                    ++it;
                }
                else
                {
                    break;
                }
            }
        }
        while (it2 != list.constEnd());

        list2.append(DateRange(first, second));
    }

/*
        for (it = list2.begin() ; it != list2.end(); ++it)
        {
            qCDebug(DIGIKAM_GENERAL_LOG) << (*it).first.date().toString(Qt::ISODate) << " :: "
                                         << (*it).second.date().toString(Qt::ISODate);
        }
*/

    return list2;
}

void TimeLineWidget::slotDatesHash(const QHash<QDateTime, int>& datesStatHash)
{
    // Clear all counts in all stats maps before to update it. Do not clear selections.

    QMap<int, Private::StatPair>::iterator it_iP;

    for (it_iP = d->yearStatMap.begin() ; it_iP != d->yearStatMap.end() ; ++it_iP)
    {
        it_iP.value().first = 0;
    }

    QMap<Private::YearRefPair, Private::StatPair>::iterator it_YP;

    for (it_YP = d->monthStatMap.begin() ; it_YP != d->monthStatMap.end() ; ++it_YP)
    {
        it_YP.value().first = 0;
    }

    for (it_YP = d->weekStatMap.begin() ; it_YP != d->weekStatMap.end() ; ++it_YP)
    {
        it_YP.value().first = 0;
    }

    for (it_YP = d->dayStatMap.begin() ; it_YP != d->dayStatMap.end() ; ++it_YP)
    {
        it_YP.value().first = 0;
    }

    // Parse all new Date stamp and store histogram stats relevant in maps.

    int count;
    QHash<QDateTime, int>::const_iterator it;

    if (datesStatHash.isEmpty())
    {
        d->minDateTime = QDateTime();
        d->maxDateTime = QDateTime();
    }
    else
    {
        d->minDateTime = datesStatHash.begin().key();
        d->maxDateTime = datesStatHash.begin().key();
    }

    for (it = datesStatHash.begin(); it != datesStatHash.end() ; ++it)
    {
        if (it.key() > d->maxDateTime)
        {
            d->maxDateTime = it.key();
        }

        if (it.key() < d->minDateTime)
        {
            d->minDateTime = it.key();
        }

        int year        = it.key().date().year();
        int month       = it.key().date().month();
        int day         = it.key().date().dayOfYear();
        int yearForWeek = year;  // Used with week shared between 2 years decade (Dec/Jan).

        int week  = it.key().date().weekNumber(&yearForWeek);

        // Stats Years values.

        it_iP = d->yearStatMap.find(year);

        if (it_iP == d->yearStatMap.end())
        {
            count = it.value();
            d->yearStatMap.insert(year, Private::StatPair(count, Unselected));
        }
        else
        {
            it_iP.value().first += it.value();
            count                = it_iP.value().first;
        }

        if (d->maxCountByYear < count)
        {
            d->maxCountByYear = count;
        }

        // Stats Months values.

        it_YP = d->monthStatMap.find(Private::YearRefPair(year, month));

        if (it_YP == d->monthStatMap.end())
        {
            count = it.value();
            d->monthStatMap.insert(Private::YearRefPair(year, month),
                                   Private::StatPair(count, Unselected));
        }
        else
        {
            it_YP.value().first += it.value();
            count                = it_YP.value().first;
        }

        if (d->maxCountByMonth < count)
        {
            d->maxCountByMonth = count;
        }

        // Stats Weeks values.

        it_YP = d->weekStatMap.find(Private::YearRefPair(yearForWeek, week));

        if (it_YP == d->weekStatMap.end())
        {
            count = it.value();
            d->weekStatMap.insert(Private::YearRefPair(yearForWeek, week),
                                  Private::StatPair(count, Unselected));
        }
        else
        {
            it_YP.value().first += it.value();
            count                = it_YP.value().first;
        }

        if (d->maxCountByWeek < count)
        {
            d->maxCountByWeek = count;
        }

        // Stats Days values.

        it_YP = d->dayStatMap.find(Private::YearRefPair(year, day));

        if (it_YP == d->dayStatMap.end())
        {
            count = it.value();
            d->dayStatMap.insert(Private::YearRefPair(year, day),
                                 Private::StatPair(count, Unselected));
        }
        else
        {
            it_YP.value().first += it.value();
            count                = it_YP.value().first;
        }

        if (d->maxCountByDay < count)
        {
            d->maxCountByDay = count;
        }
    }

    if (!datesStatHash.isEmpty())
    {
        d->maxDateTime.setTime(QTime(0, 0, 0, 0));
        d->minDateTime.setTime(QTime(0, 0, 0, 0));
    }
    else
    {
        d->maxDateTime = d->refDateTime;
        d->minDateTime = d->refDateTime;
    }

    update();
    Q_EMIT signalDateMapChanged();
}

int TimeLineWidget::calculateTop(int& val) const
{
    const int minimum_valid_height = 1;
    double max                     = (double)maxCount();
    int dim                        = height() - d->bottomMargin - d->topMargin;

    if (d->scaleMode == TimeLineWidget::LogScale)
    {
        if (max > 0.0)
        {
            max = log(max);
        }
        else
        {
            max = 1.0;
        }

        double logVal = 0;

        if (val <= 0)
        {
            logVal = 0;
        }
        else
        {
            logVal = log(val);
        }

        int pix = lround((logVal * dim) / max);

        if (val)
        {
            pix = qMax(pix, minimum_valid_height);
        }

        int top = dim + d->topMargin - pix;

        if (top < 0)
        {
            val = 0;
        }

        return top;
    }
    else
    {
        int pix = lround((val * dim) / max);

        if (val)
        {
            pix = qMax(pix, minimum_valid_height);
        }

        int top = dim + d->topMargin - pix;

        return top;
    }
}

void TimeLineWidget::paintItem(QPainter& p, const QRect& barRect,
                               const QDateTime& ref, const int& separatorPosition,
                               const QColor& dateColor, const QColor& subDateColor)
{
    switch (d->timeUnit)
    {
        case Day:
        {
            {
                p.save();
                QFont fnt = p.font();
                fnt.setPointSize(fnt.pointSize() - 4);
                p.setFont(fnt);
                p.setPen(subDateColor);
                QString txt = QLocale().standaloneDayName(ref.date().day(), QLocale::ShortFormat);
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);
                p.drawText(barRect.left() + ((barRect.width() - br.width()) / 2),
                           barRect.bottom() + br.height(), txt);
                p.restore();
            }

            if (ref.date().day() == 1)
            {
                p.setPen(dateColor);
                p.drawLine(barRect.left(), barRect.bottom(),
                           barRect.left(), barRect.bottom() + d->bottomMargin / 2);
                QString txt = QLocale().toString(ref.date(), QLocale::ShortFormat);
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);
                p.drawText(barRect.left() - br.width() / 2, barRect.bottom() + d->bottomMargin, txt);
            }

            break;
        }

        case Week:
        {
            int week = ref.date().weekNumber();

            {
                p.save();
                QFont fnt = p.font();
                fnt.setPointSize(fnt.pointSize() - 4);
                p.setFont(fnt);
                p.setPen(subDateColor);
                QString txt = QString::number(week);
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);
                p.drawText(barRect.left() + ((barRect.width() - br.width()) / 2),
                           barRect.bottom() + br.height(), txt);
                p.restore();
            }

            p.setPen(dateColor);

            if      ((week == 1) || (week == 10) || (week == 20) || (week == 30) || (week == 40) || (week == 50))
            {
                p.drawLine(barRect.left(), barRect.bottom(),
                           barRect.left(), barRect.bottom() + d->bottomMargin / 2);
                QString txt = QLocale().toString(ref.date(), QLocale::ShortFormat);
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);

                if (week != 50)
                {
                    p.drawText(barRect.left() - br.width() / 2, barRect.bottom() + d->bottomMargin, txt);
                }
            }
            else if ((week == 6) || (week == 16) || (week == 26) || (week == 36) || (week == 46))
            {
                p.drawLine(barRect.left(), barRect.bottom(),
                           barRect.left(), barRect.bottom() + d->bottomMargin / 4);
            }

            break;
        }

        case Month:
        {
            {
                p.save();
                QFont fnt = p.font();
                fnt.setPointSize(fnt.pointSize() - 4);
                p.setFont(fnt);
                p.setPen(subDateColor);
                QString txt = QLocale().standaloneMonthName(ref.date().month(), QLocale::ShortFormat);
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);
                p.drawText(barRect.left() + ((barRect.width() - br.width()) / 2),
                           barRect.bottom() + br.height(), txt);
                p.restore();
            }

            p.setPen(dateColor);

            if      (ref.date().month() == 1)
            {
                p.drawLine(barRect.left(), barRect.bottom(),
                           barRect.left(), barRect.bottom() + d->bottomMargin / 2);
                QString txt = QString::number(ref.date().year());
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);
                p.drawText(barRect.left() - br.width() / 2, barRect.bottom() + d->bottomMargin, txt);
            }
            else if (ref.date().month() == 7)
            {
                p.drawLine(separatorPosition, barRect.bottom(),
                           separatorPosition, barRect.bottom() + d->bottomMargin / 4);
            }

            break;
        }

        case Year:
        {
            p.setPen(dateColor);

            if      (ref.date().year() % 10 == 0)
            {
                p.drawLine(barRect.left(), barRect.bottom(),
                           barRect.left(), barRect.bottom() + d->bottomMargin / 2);
                QString txt = QString::number(ref.date().year());
                QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt);
                p.drawText(barRect.left() - br.width() / 2, barRect.bottom() + d->bottomMargin, txt);
            }
            else if (ref.date().year() % 5 == 0)
                p.drawLine(separatorPosition, barRect.bottom(),
                           separatorPosition, barRect.bottom() + d->bottomMargin / 4);

            break;
        }
    }
}

void TimeLineWidget::keyPressEvent(QKeyEvent *e)
{
    QDateTime ref       = d->cursorDateTime;
    bool isScrollNext;

    bool ctrlPressed    = e->modifiers() & Qt::ControlModifier;
    bool shiftPressed   = e->modifiers() & Qt::ShiftModifier;

    if (!ctrlPressed && !shiftPressed)
    {
        resetSelection();
    }

    if (!shiftPressed)
    {
        d->selStartDateTime = ref;
        d->selMinDateTime   = ref;
        d->selMaxDateTime   = ref;
    }

    update();

    switch (e->key())
    {
        case Qt::Key_J:
        {
            QDateTime dt = prevDateTime(ref);
            setDateTimeSelected(dt, Selected);

            isScrollNext = false;
            keyScroll(isScrollNext);

            ref = nextDateTime(ref);

            if (!dt.isNull())
            {
                setCursorDateTime(dt);
            }

            break;
        }

        case Qt::Key_K:
        {
            QDateTime dt = nextDateTime(ref);

            setDateTimeSelected(dt, Selected);

            isScrollNext = true;
            keyScroll(isScrollNext); // passed true to scroll to the next item.

            ref = prevDateTime(ref);

            if (!dt.isNull())
            {
                setCursorDateTime(dt);
            }

            break;
        }

        default:
        {
            e->ignore();
            break;
        }
    }
}

void TimeLineWidget::keyReleaseEvent(QKeyEvent *)
{
    updateAllSelection();
    Q_EMIT signalSelectionChanged();
    update();
}

void TimeLineWidget::keyScroll(bool isScrollNext)
{
    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect barRect;
    QRect deskRect  = screen->geometry();
    int items       = deskRect.width() / d->barWidth;

    d->nbItems      = (int)((width() / 2.0) / (float)d->barWidth);
    d->startPos     = (int)((width() / 2.0) - ((float)(d->barWidth) / 2.0));

    if (isScrollNext)
    {
        barRect.setTop(0);
        barRect.setBottom(height() - d->bottomMargin + 1);

        for (int i = 0 ; i < items ; ++i)
        {
            barRect.setLeft(d->startPos + i * d->barWidth);
            barRect.setRight(d->startPos + (i + 1)*d->barWidth);

            if (barRect.intersects(d->focusRect) && i >= d->nbItems)
            {
                // Scroll to the next item. Because the focus rect is outside the visible area.

                slotNext();
                break;
            }
        }
    }
    else
    {
        barRect.setTop(0);
        barRect.setBottom(height() - d->bottomMargin + 1);

        for (int i = 0 ; i < items ; ++i)
        {
            barRect.setRight(d->startPos - i * d->barWidth);
            barRect.setLeft(d->startPos - (i + 1)*d->barWidth);

            if (barRect.intersects(d->focusRect) && (i >= (d->nbItems - 1)))
            {
                // Scroll to the previous item. Because the focus rect is outside the visible area.

                slotPrevious();
                break;
            }
        }
    }
}

void TimeLineWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    d->bottomMargin = (int)(p.fontMetrics().height() * 1.5);
    d->barWidth     = p.fontMetrics().horizontalAdvance(QLatin1String("00"));
    d->nbItems      = (int)((width() / 2.0) / (float)d->barWidth);
    d->startPos     = (int)((width() / 2.0) - ((float)(d->barWidth) / 2.0));
    QDateTime     ref;
    int           val, top;
    SelectionMode sel;
    QRect         focusRect, selRect, barRect;
    QBrush        selBrush;
    QColor        dateColor, subDateColor;

    // Date histogram drawing is divided in 2 parts. The current date-time
    // is placed on the center of the view and all dates on right are computed,
    // and in second time, all dates on the left.

    // Draw all dates on the right of ref. date-time.

    ref = d->refDateTime;
    ref.setTime(QTime(0, 0, 0, 0));

    for (int i = 0 ; i < d->nbItems ; ++i)
    {
        val = statForDateTime(ref, sel);
        top = calculateTop(val);

        barRect.setTop(top);
        barRect.setLeft(d->startPos + i * d->barWidth);
        barRect.setBottom(height() - d->bottomMargin);
        barRect.setRight(d->startPos + (i + 1)*d->barWidth);

        if (ref == d->cursorDateTime)
        {
            focusRect = barRect;
        }

        if (ref > d->maxDateTime)
        {
            dateColor = palette().color(QPalette::Active, QPalette::Mid);
        }
        else
        {
            dateColor = palette().color(QPalette::WindowText);
        }

        p.setPen(palette().color(QPalette::WindowText));
        p.fillRect(barRect, QBrush(qApp->palette().color(QPalette::Link)));
        p.drawLine(barRect.topLeft(), barRect.topRight());
        p.drawLine(barRect.topRight(), barRect.bottomRight());
        p.drawLine(barRect.bottomRight(), barRect.bottomLeft());
        p.drawLine(barRect.bottomLeft(), barRect.topLeft());
        p.drawLine(barRect.right(), barRect.bottom(), barRect.right(), barRect.bottom() + 3);
        p.drawLine(barRect.left(),  barRect.bottom(), barRect.left(),  barRect.bottom() + 3);

        if (val)
        {
            if (sel)
            {
                subDateColor = palette().color(QPalette::Active, QPalette::HighlightedText);
            }
            else
            {
                subDateColor = palette().color(QPalette::Active, QPalette::WindowText);
            }
        }
        else
        {
            subDateColor = palette().color(QPalette::Active, QPalette::Mid);
        }

        if ((sel == Selected) || (sel == FuzzySelection))
        {
            selBrush.setColor(qApp->palette().color(QPalette::Highlight));
            selBrush.setStyle(Qt::SolidPattern);

            if (sel == FuzzySelection)
            {
                selBrush.setStyle(Qt::Dense4Pattern);
            }

            selRect.setTop(height() - d->bottomMargin + 1);
            selRect.setLeft(d->startPos + i * d->barWidth);
            selRect.setBottom(height() - d->bottomMargin / 2);
            selRect.setRight(d->startPos + (i + 1)*d->barWidth);
            p.fillRect(selRect, selBrush);

            p.fillRect(barRect, selBrush);
            p.drawLine(barRect.topLeft(), barRect.topRight());
            p.drawLine(barRect.topRight(), barRect.bottomRight());
            p.drawLine(barRect.bottomRight(), barRect.bottomLeft());
            p.drawLine(barRect.bottomLeft(), barRect.topLeft());
            p.drawLine(barRect.right(), barRect.bottom(), barRect.right(), barRect.bottom() + 3);
            p.drawLine(barRect.left(),  barRect.bottom(), barRect.left(),  barRect.bottom() + 3);
        }

        paintItem(p, barRect, ref, barRect.left(), dateColor, subDateColor);

        ref = nextDateTime(ref);
    }

    // Draw all dates on the left of ref. date-time.

    ref = d->refDateTime;
    ref.setTime(QTime(0, 0, 0, 0));
    ref = prevDateTime(ref);

    for (int i = 0 ; i < d->nbItems - 1 ; ++i)
    {
        val = statForDateTime(ref, sel);
        top = calculateTop(val);

        barRect.setTop(top);
        barRect.setRight(d->startPos - i * d->barWidth);
        barRect.setBottom(height() - d->bottomMargin);
        barRect.setLeft(d->startPos - (i + 1)*d->barWidth);

        if (ref == d->cursorDateTime)
        {
            focusRect = barRect;
        }

        if (ref < d->minDateTime)
        {
            dateColor = palette().color(QPalette::Active, QPalette::Mid);
        }
        else
        {
            dateColor = palette().color(QPalette::WindowText);
        }

        p.setPen(palette().color(QPalette::WindowText));
        p.fillRect(barRect, QBrush(qApp->palette().color(QPalette::Link)));
        p.drawLine(barRect.topLeft(), barRect.topRight());
        p.drawLine(barRect.topRight(), barRect.bottomRight());
        p.drawLine(barRect.bottomRight(), barRect.bottomLeft());
        p.drawLine(barRect.bottomLeft(), barRect.topLeft());
        p.drawLine(barRect.right(), barRect.bottom(), barRect.right(), barRect.bottom() + 3);
        p.drawLine(barRect.left(),  barRect.bottom(), barRect.left(),  barRect.bottom() + 3);

        if (val)
        {
            if (sel)
            {
                subDateColor = palette().color(QPalette::Active, QPalette::HighlightedText);
            }
            else
            {
                subDateColor = palette().color(QPalette::Active, QPalette::WindowText);
            }
        }
        else
        {
            subDateColor = palette().color(QPalette::Active, QPalette::Mid);
        }

        if ((sel == Selected) || (sel == FuzzySelection))
        {
            selBrush.setColor(qApp->palette().color(QPalette::Highlight));
            selBrush.setStyle(Qt::SolidPattern);

            if (sel == FuzzySelection)
            {
                selBrush.setStyle(Qt::Dense4Pattern);
            }

            selRect.setTop(height() - d->bottomMargin + 1);
            selRect.setLeft(d->startPos - (i + 1)*d->barWidth);
            selRect.setBottom(height() - d->bottomMargin / 2);
            selRect.setRight(d->startPos - i * d->barWidth);
            p.fillRect(selRect, selBrush);

            p.fillRect(barRect, selBrush);
            p.drawLine(barRect.topLeft(), barRect.topRight());
            p.drawLine(barRect.topRight(), barRect.bottomRight());
            p.drawLine(barRect.bottomRight(), barRect.bottomLeft());
            p.drawLine(barRect.bottomLeft(), barRect.topLeft());
            p.drawLine(barRect.right(), barRect.bottom(), barRect.right(), barRect.bottom() + 3);
            p.drawLine(barRect.left(),  barRect.bottom(), barRect.left(),  barRect.bottom() + 3);
        }

        paintItem(p, barRect, ref, barRect.right(), dateColor, subDateColor);

        ref = prevDateTime(ref);
    }

    // Draw focus rectangle over current date-time.

    if (focusRect.isValid())
    {
        focusRect.setTop(d->topMargin);
        QPoint p1(focusRect.left(), height() - d->bottomMargin);
        QPoint p2(focusRect.right(), height() - d->bottomMargin);
        focusRect.setBottom(focusRect.bottom() + d->bottomMargin / 2);
        d->focusRect = focusRect;

        p.setPen(palette().color(QPalette::Active, QPalette::Shadow));
        p.drawLine(p1.x(), p1.y() + 1, p2.x(), p2.y() + 1);
        p.drawLine(focusRect.topLeft(), focusRect.topRight());
        p.drawLine(focusRect.topRight(), focusRect.bottomRight());
        p.drawLine(focusRect.bottomRight(), focusRect.bottomLeft());
        p.drawLine(focusRect.bottomLeft(), focusRect.topLeft());

        p.setPen(palette().color(QPalette::Active, QPalette::Shadow));
        focusRect.adjust(-1, -1, 1, 1);
        p.setPen(qApp->palette().color(QPalette::HighlightedText));
        p.drawLine(focusRect.topLeft(), focusRect.topRight());
        p.drawLine(focusRect.topRight(), focusRect.bottomRight());
        p.drawLine(focusRect.bottomRight(), focusRect.bottomLeft());
        p.drawLine(focusRect.bottomLeft(), focusRect.topLeft());
        p.drawLine(p1.x() - 1, p1.y(), p2.x() + 1, p2.y());

        focusRect.adjust(-1, -1, 1, 1);
        p.drawLine(focusRect.topLeft(), focusRect.topRight());
        p.drawLine(focusRect.topRight(), focusRect.bottomRight());
        p.drawLine(focusRect.bottomRight(), focusRect.bottomLeft());
        p.drawLine(focusRect.bottomLeft(), focusRect.topLeft());
        p.drawLine(p1.x() - 1, p1.y() - 1, p2.x() + 1, p2.y() - 1);

        focusRect.adjust(-1, -1, 1, 1);
        p.setPen(palette().color(QPalette::Active, QPalette::Shadow));
        p.drawLine(focusRect.topLeft(), focusRect.topRight());
        p.drawLine(focusRect.topRight(), focusRect.bottomRight());
        p.drawLine(focusRect.bottomRight(), focusRect.bottomLeft());
        p.drawLine(focusRect.bottomLeft(), focusRect.topLeft());
        p.drawLine(p1.x(), p1.y() - 2, p2.x(), p2.y() - 2);
    }

    p.end();
}

QDateTime TimeLineWidget::prevDateTime(const QDateTime& dt) const
{
    QDateTime prev;

    switch (d->timeUnit)
    {
        case Day:
        {
            prev = dt.addDays(-1);
            break;
        }

        case Week:
        {
            prev = dt.addDays(-7);
            break;
        }

        case Month:
        {
            prev = dt.addMonths(-1);
            break;
        }

        case Year:
        {
            prev = dt.addYears(-1);
            break;
        }
    }

    return prev;
}

QDateTime TimeLineWidget::nextDateTime(const QDateTime& dt) const
{
    QDateTime next;

    switch (d->timeUnit)
    {
        case Day:
        {
            next = dt.addDays(1);
            break;
        }

        case Week:
        {
            next = dt.addDays(7);
            break;
        }

        case Month:
        {
            next = dt.addMonths(1);
            break;
        }

        case Year:
        {
            next = dt.addYears(1);
            break;
        }
    }

    return next;
}

int TimeLineWidget::maxCount() const
{
    int max = 1;

    switch (d->timeUnit)
    {
        case Day:
        {
            max = d->maxCountByDay;
            break;
        }

        case Week:
        {
            max = d->maxCountByWeek;
            break;
        }

        case Month:
        {
            max = d->maxCountByMonth;
            break;
        }

        case Year:
        {
            max = d->maxCountByYear;
            break;
        }
    }

    return max;
}

int TimeLineWidget::statForDateTime(const QDateTime& dt, SelectionMode& selected) const
{
    int count        = 0;
    int year         = dt.date().year();
    int month        = dt.date().month();
    int day          = dt.date().dayOfYear();
    int yearForWeek  = year;  // Used with week shared between 2 years decade (Dec/Jan).

    int week         = dt.date().weekNumber(&yearForWeek);

    selected         = Unselected;

    QMap<Private::YearRefPair, Private::StatPair>::const_iterator it;

    switch (d->timeUnit)
    {
        case Day:
        {
            it = d->dayStatMap.constFind(Private::YearRefPair(year, day));

            if (it != d->dayStatMap.constEnd())
            {
                count     = it.value().first;
                selected  = it.value().second;
            }

            break;
        }

        case Week:
        {
            it = d->weekStatMap.constFind(Private::YearRefPair(yearForWeek, week));

            if (it != d->weekStatMap.constEnd())
            {
                count     = it.value().first;
                selected  = it.value().second;
            }

            break;
        }

        case Month:
        {
            it = d->monthStatMap.constFind(Private::YearRefPair(year, month));

            if (it != d->monthStatMap.constEnd())
            {
                count     = it.value().first;
                selected  = it.value().second;
            }

            break;
        }

        case Year:
        {
            QMap<int, Private::StatPair>::const_iterator it2 = d->yearStatMap.constFind(year);

            if (it2 != d->yearStatMap.constEnd())
            {
                count     = it2.value().first;
                selected  = it2.value().second;
            }

            break;
        }
    }

    return count;
}

void TimeLineWidget::setDateTimeSelected(const QDateTime& dt, SelectionMode selected)
{
    int year        = dt.date().year();
    int month       = dt.date().month();
    int yearForWeek = year;  // Used with week shared between 2 years decade (Dec/Jan).

    int week        = dt.date().weekNumber(&yearForWeek);

    QDateTime dts, dte;

    switch (d->timeUnit)
    {
        case Day:
        {
            dts = dt;
            dte = dts.addDays(1);
            setDaysRangeSelection(dts, dte, selected);
            break;
        }

        case Week:
        {
            dts = firstDayOfWeek(yearForWeek, week);
            dte = dts.addDays(7);
            setDaysRangeSelection(dts, dte, selected);
            updateWeekSelection(dts, dte);
            break;
        }

        case Month:
        {
            dts = startOfDay(QDate(year, month, 1));
            dte = dts.addDays(dts.date().daysInMonth());
            setDaysRangeSelection(dts, dte, selected);
            updateMonthSelection(dts, dte);
            break;
        }

        case Year:
        {
            dts = startOfDay(QDate(year, 1, 1));
            dte = dts.addDays(dts.date().daysInYear());
            setDaysRangeSelection(dts, dte, selected);
            updateYearSelection(dts, dte);
            break;
        }
    }
}

void TimeLineWidget::updateWeekSelection(const QDateTime& dts, const QDateTime& dte)
{
    QMap<Private::YearRefPair, Private::StatPair>::iterator it;
    QDateTime dtsWeek, dteWeek, dt;
    int week;
    int yearForWeek;  // Used with week shared between 2 years decade (Dec/Jan).
    dt = dts;

    do
    {
        yearForWeek = dt.date().year();
        week        = dt.date().weekNumber(&yearForWeek);
        dtsWeek     = firstDayOfWeek(yearForWeek, week);
        dteWeek     = dtsWeek.addDays(7);
        it          = d->weekStatMap.find(Private::YearRefPair(yearForWeek, week));

        if (it != d->weekStatMap.end())
        {
            it.value().second = checkSelectionForDaysRange(dtsWeek, dteWeek);
        }

        dt = dt.addDays(7);
    }
    while (dt <= dte);
}

void TimeLineWidget::updateMonthSelection(const QDateTime& dts, const QDateTime& dte)
{
    QMap<Private::YearRefPair, Private::StatPair>::iterator it;
    QDateTime dtsMonth, dteMonth, dt;
    int year, month;
    dt = dts;

    do
    {
        year     = dt.date().year();
        month    = dt.date().month();
        dtsMonth = startOfDay(QDate(year, month, 1));
        dteMonth = dtsMonth.addDays(dtsMonth.date().daysInMonth());
        it       = d->monthStatMap.find(Private::YearRefPair(year, month));

        if (it != d->monthStatMap.end())
        {
            it.value().second = checkSelectionForDaysRange(dtsMonth, dteMonth);
        }

        dt = dteMonth;
    }
    while (dt <= dte);
}

void TimeLineWidget::updateYearSelection(const QDateTime& dts, const QDateTime& dte)
{
    QMap<int, Private::StatPair>::iterator it;
    QDateTime dtsYear, dteYear, dt;
    int year;
    dt = dts;

    do
    {
        year    = dt.date().year();
        dtsYear = startOfDay(QDate(year, 1, 1));
        dteYear = dtsYear.addDays(dtsYear.date().daysInYear());
        it      = d->yearStatMap.find(year);

        if (it != d->yearStatMap.end())
        {
            it.value().second = checkSelectionForDaysRange(dtsYear, dteYear);
        }

        dt = dteYear;
    }
    while (dt <= dte);
}

void TimeLineWidget::updateAllSelection()
{
    QMap<Private::YearRefPair, Private::StatPair>::const_iterator it;
    QDateTime dts, dte;
    QDate     date;

    for (it = d->dayStatMap.constBegin() ; it != d->dayStatMap.constEnd() ; ++it)
    {
        if (it.value().second == Selected)
        {
            date = QDate(it.key().first, 1, 1);
            date = date.addDays(it.key().second - 1);
            dts  = startOfDay(date);
            dte  = dts.addDays(1);
            updateWeekSelection(dts, dte);
            updateMonthSelection(dts, dte);
            updateYearSelection(dts, dte);
        }
    }
}

void TimeLineWidget::setDaysRangeSelection(const QDateTime& dts, const QDateTime& dte, SelectionMode selected)
{
    int year, day;
    QDateTime dt = dts;
    QMap<Private::YearRefPair, Private::StatPair>::iterator it;

    do
    {
        year = dt.date().year();
        day  = dt.date().dayOfYear();
        it   = d->dayStatMap.find(Private::YearRefPair(year, day));

        if (it != d->dayStatMap.end())
        {
            it.value().second = selected;
        }

        dt = dt.addDays(1);
    }
    while (dt < dte);
}

TimeLineWidget::SelectionMode TimeLineWidget::checkSelectionForDaysRange(const QDateTime& dts, const QDateTime& dte) const
{
    int year, day;
    int items    = 0;
    int itemsFuz = 0;
    int itemsSel = 0;
    QDateTime dt = dts;
    QMap<Private::YearRefPair, Private::StatPair>::iterator it;

    do
    {
        year = dt.date().year();
        day  = dt.date().dayOfYear();
        it   = d->dayStatMap.find(Private::YearRefPair(year, day));

        if (it != d->dayStatMap.end())
        {
            ++items;

            if (it.value().second != Unselected)
            {
                if (it.value().second == FuzzySelection)
                {
                    ++itemsFuz;
                }
                else
                {
                    ++itemsSel;
                }
            }
        }

        dt = dt.addDays(1);
    }
    while (dt < dte);

    if (items == 0)
    {
        return Unselected;
    }

    if ((itemsFuz == 0) && (itemsSel == 0))
    {
        return Unselected;
    }

    if (itemsFuz > 0)
    {
        return FuzzySelection;
    }

    if (items > itemsSel)
    {
        return FuzzySelection;
    }

    return Selected;
}

void TimeLineWidget::slotBackward()
{
    QDateTime ref = d->refDateTime;

    switch (d->timeUnit)
    {
        case Day:
        {
            for (int i = 0 ; i < 7 ; ++i)
            {
                ref = prevDateTime(ref);
            }

            break;
        }

        case Week:
        {
            for (int i = 0 ; i < 4 ; ++i)
            {
                ref = prevDateTime(ref);
            }

            break;
        }

        case Month:
        {
            for (int i = 0 ; i < 12 ; ++i)
            {
                ref = prevDateTime(ref);
            }

            break;
        }

        case Year:
        {
            for (int i = 0 ; i < 5 ; ++i)
            {
                ref = prevDateTime(ref);
            }

            break;
        }
    }

    if (ref < d->minDateTime)
    {
        ref = d->minDateTime;
    }

    setRefDateTime(ref);
}

void TimeLineWidget::slotPrevious()
{
    if (d->refDateTime <= d->minDateTime)
    {
        return;
    }

    QDateTime ref = prevDateTime(d->refDateTime);
    setRefDateTime(ref);
}

void TimeLineWidget::slotNext()
{
    if (d->refDateTime >= d->maxDateTime)
    {
        return;
    }

    QDateTime ref = nextDateTime(d->refDateTime);
    setRefDateTime(ref);
}

void TimeLineWidget::slotForward()
{
    QDateTime ref = d->refDateTime;

    switch (d->timeUnit)
    {
        case Day:
        {
            for (int i = 0 ; i < 7 ; ++i)
            {
                ref = nextDateTime(ref);
            }

            break;
        }

        case Week:
        {
            for (int i = 0 ; i < 4 ; ++i)
            {
                ref = nextDateTime(ref);
            }

            break;
        }

        case Month:
        {
            for (int i = 0; i < 12; ++i)
            {
                ref = nextDateTime(ref);
            }

            break;
        }

        case Year:
        {
            for (int i = 0 ; i < 5 ; ++i)
            {
                ref = nextDateTime(ref);
            }

            break;
        }
    }

    if (ref > d->maxDateTime)
    {
        ref = d->maxDateTime;
    }

    setRefDateTime(ref);
}

void TimeLineWidget::wheelEvent(QWheelEvent* e)
{
    if (e->angleDelta().y() < 0)
    {
        if (e->modifiers() & Qt::ShiftModifier)
        {
            slotForward();
        }
        else
        {
            slotNext();
        }
    }

    if (e->angleDelta().y() > 0)
    {
        if (e->modifiers() & Qt::ShiftModifier)
        {
            slotBackward();
        }
        else
        {
            slotPrevious();
        }
    }
}

void TimeLineWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        QPoint pt(e->position().toPoint().x(), e->position().toPoint().y());
#else
        QPoint pt(e->x(), e->y());
#endif

        bool ctrlPressed    = e->modifiers() & Qt::ControlModifier;
        bool shiftPressed   = e->modifiers() & Qt::ShiftModifier;
        QDateTime ref       = dateTimeForPoint(pt, d->selMouseEvent);

        if (d->selMouseEvent)
        {
            if (!ctrlPressed && !shiftPressed)
            {
                resetSelection();
            }

            setDateTimeSelected(ref, Selected);

            if (!shiftPressed)
            {
                d->selStartDateTime = ref;
                d->selMinDateTime   = ref;
                d->selMaxDateTime   = ref;
            }
            else
            {
                bool sel;
                QDateTime selEndDateTime = dateTimeForPoint(pt, sel);
                setCursorDateTime(selEndDateTime);
                handleSelectionRange(selEndDateTime);
            }
        }

        if (!ref.isNull())
        {
            setCursorDateTime(ref);
        }

        d->validMouseEvent = true;
        setFocus();
        update();
    }
}

void TimeLineWidget::mouseMoveEvent(QMouseEvent* e)
{
    // set cursor shape to indicate selection area

    QRect selectionArea;
    selectionArea.setTop(d->topMargin);
    selectionArea.setLeft(0);
    selectionArea.setBottom(height());
    selectionArea.setRight(width());

    bool      sel;
    QDateTime selEndDateTime;

    if (selectionArea.contains(e->pos()))
    {
        SelectionMode unused;
        selEndDateTime          = dateTimeForPoint(e->pos(), sel);
        bool hasSelectableDates = statForDateTime(selEndDateTime, unused);

        if (hasSelectableDates)
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            unsetCursor();
        }
    }
    else
    {
        unsetCursor();
    }

    // handle move event

    if (d->validMouseEvent == true)
    {
#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        QPoint pt(e->position().toPoint().x(), e->position().toPoint().y());
#else
        QPoint pt(e->x(), e->y());
#endif

        if (selEndDateTime.isNull())
        {
            selEndDateTime = dateTimeForPoint(pt, sel);
        }

        setCursorDateTime(selEndDateTime);

        handleSelectionRange(selEndDateTime);
        update();
    }
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent*)
{
    d->validMouseEvent = false;
    d->slotNextTimer->stop();
    d->slotPreviousTimer->stop();

    // Only dispatch changes about selection when user release mouse selection
    // to prevent multiple queries on database.

    if (d->selMouseEvent)
    {
        updateAllSelection();
        Q_EMIT signalSelectionChanged();
    }

    d->selMouseEvent = false;
}

QDateTime TimeLineWidget::dateTimeForPoint(const QPoint& pt, bool& isOnSelectionArea)
{
    QRect barRect;
    isOnSelectionArea = false;

    // Check on the right of reference date.

    QDateTime ref = d->refDateTime;
    ref.setTime(QTime(0, 0, 0, 0));

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = nativeParentWidget())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    QRect deskRect = screen->geometry();
    int items      = deskRect.width() / d->barWidth;

    for (int i = 0 ; i < items ; ++i)
    {
        barRect.setTop(0);
        barRect.setLeft(d->startPos + i * d->barWidth);
        barRect.setBottom(height() - d->bottomMargin + 1);
        barRect.setRight(d->startPos + (i + 1)*d->barWidth);

        if (barRect.contains(pt))
        {
            isOnSelectionArea = true;

            if (i >= d->nbItems)
            {
                // Point is outside visible widget area. We scrolling widget contents.

                if (d->validMouseEvent)
                {
                    d->slotNextTimer->start();
                }
            }
            else
            {
                d->slotNextTimer->stop();
            }

            return ref;
        }

        ref = nextDateTime(ref);
    }

    // Check on the left of reference date.

    ref = d->refDateTime;
    ref.setTime(QTime(0, 0, 0, 0));
    ref = prevDateTime(ref);

    for (int i = 0 ; i < items ; ++i)
    {
        barRect.setTop(0);
        barRect.setRight(d->startPos - i * d->barWidth);
        barRect.setBottom(height() - d->bottomMargin + 1);
        barRect.setLeft(d->startPos - (i + 1)*d->barWidth);

        if (barRect.contains(pt))
        {
            isOnSelectionArea = true;

            if (i >= (d->nbItems - 1))
            {
                // Point is outside visible widget area. We scrolling widget contents.

                if (d->validMouseEvent)
                {
                    d->slotPreviousTimer->start();
                }
            }
            else
            {
                d->slotPreviousTimer->stop();
            }

            return ref;
        }

        ref = prevDateTime(ref);
    }

    return QDateTime();
}

QDateTime TimeLineWidget::firstDayOfWeek(int year, int weekNumber) const
{
    // Search the first day of first week of year.
    // We start to scan from 1st December of year-1 because
    // first week of year OR last week of year-1 can be shared
    // between year-1 and year.

    QDateTime d1 = startOfDay(QDate(year - 1, 12, 1));
    QDateTime dt = d1;
    int weekYear = dt.date().year();
    int weekNum  = 0;

    do
    {
        dt      = dt.addDays(1);
        weekNum = dt.date().weekNumber(&weekYear);
    }
    while ((weekNum != 1) && (weekYear != year));

    dt = dt.addDays((weekNumber - 1) * 7);

    /*
        qCDebug(DIGIKAM_GENERAL_LOG) << "Year= " << year << " Week= " << weekNumber
                                     << " 1st day= " << dt;
    */

    return dt;
}

void TimeLineWidget::slotThemeChanged()
{
    update();
}

void TimeLineWidget::handleSelectionRange(const QDateTime& selEndDateTime)
{
    // Clamp start and end date-time of current contiguous selection.

    if (!selEndDateTime.isNull() && !d->selStartDateTime.isNull())
    {
        if      ((selEndDateTime > d->selStartDateTime) &&
                 (selEndDateTime > d->selMaxDateTime))
        {
            d->selMaxDateTime = selEndDateTime;
        }
        else if ((selEndDateTime < d->selStartDateTime) &&
                 (selEndDateTime < d->selMinDateTime))
        {
            d->selMinDateTime = selEndDateTime;
        }

        QDateTime dt = d->selMinDateTime;

        do
        {
            setDateTimeSelected(dt, Unselected);
            dt = nextDateTime(dt);
        }
        while (dt <= d->selMaxDateTime);
    }

    // Now perform selections on Date Maps.

    if (d->selMouseEvent)
    {
        if (!d->selStartDateTime.isNull() && !selEndDateTime.isNull())
        {
            QDateTime dt = d->selStartDateTime;

            if (selEndDateTime > d->selStartDateTime)
            {
                do
                {
                    setDateTimeSelected(dt, Selected);
                    dt = nextDateTime(dt);
                }
                while (dt <= selEndDateTime);
            }
            else
            {
                do
                {
                    setDateTimeSelected(dt, Selected);
                    dt = prevDateTime(dt);
                }
                while (dt >= selEndDateTime);
            }
        }
    }
}

} // namespace Digikam
