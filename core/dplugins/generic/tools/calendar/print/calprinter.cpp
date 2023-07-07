/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-13
 * Description : printer thread.
 *
 * SPDX-FileCopyrightText: 2008      by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "calprinter.h"

// Qt includes

#include <QPrinter>

// Local includes

#include "calpainter.h"

namespace DigikamGenericCalendarPlugin
{

class Q_DECL_HIDDEN CalPrinter::Private
{
public:

    explicit Private()
      : cancelled(false),
        printer  (nullptr),
        painter  (nullptr)
    {
    }

    bool             cancelled;

    QMap<int, QUrl>  months;
    QPrinter*        printer;

    CalPainter*      painter;
};

CalPrinter::CalPrinter(QPrinter* const printer,
                       const QMap<int, QUrl>& months,
                       QObject* const parent)
    : QThread(parent),
      d      (new Private)
{
    d->printer   = printer;
    d->painter   = new CalPainter(d->printer);
    d->months    = months;
    d->cancelled = false;
}

CalPrinter::~CalPrinter()
{
    delete d->painter;
    delete d;
}

void CalPrinter::run()
{
    connect(d->painter, SIGNAL(signalTotal(int)),
            this, SIGNAL(totalBlocks(int)));

    connect(d->painter, SIGNAL(signalProgress(int)),
            this, SIGNAL(blocksFinished(int)));

    int currPage = 0;

    Q_FOREACH (const int month, d->months.keys())
    {
        Q_EMIT pageChanged(currPage);

        // cppcheck-suppress knownConditionTrueFalse
        if (currPage)
        {
            d->printer->newPage();
        }

        ++currPage;

        d->painter->setImage(d->months.value(month));
        d->painter->paint(month);

        if (d->cancelled)
        {
            break;
        }
    }

    d->painter->end();

    Q_EMIT pageChanged(currPage);
}

void CalPrinter::cancel()
{
    d->painter->cancel();
    d->cancelled = true;
}

} // namespace Digikam
