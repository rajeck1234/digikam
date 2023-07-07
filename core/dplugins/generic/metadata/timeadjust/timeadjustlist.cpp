/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-17-04
 * Description : time adjust images list.
 *
 * SPDX-FileCopyrightText: 2012      by Smit Mehta <smit dot meh at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2018-2021 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timeadjustlist.h"

// Qt includes

#include <QLocale>
#include <QHeaderView>

// KDE includes

#include <klocalizedstring.h>

namespace DigikamGenericTimeAdjustPlugin
{

TimeAdjustList::TimeAdjustList(QWidget* const parent)
    : DItemsList(parent)
{
    setControlButtonsPlacement(DItemsList::NoControlButtons);
    listView()->setColumn(static_cast<Digikam::DItemsListView::ColumnType>(TIMESTAMP_USED),
                          i18n("Timestamp Used"), true);
    listView()->setColumn(static_cast<Digikam::DItemsListView::ColumnType>(TIMESTAMP_UPDATED),
                          i18n("Timestamp Updated"), true);
    listView()->setColumn(static_cast<Digikam::DItemsListView::ColumnType>(STATUS),
                          i18n("Status"), true);

    listView()->header()->setSectionResizeMode(QHeaderView::Stretch);
    listView()->setSelectionMode(QAbstractItemView::SingleSelection);
    listView()->setUniformRowHeights(true);
}

TimeAdjustList::~TimeAdjustList()
{
}

void TimeAdjustList::setStatus(const QUrl& url,
                               const QDateTime& org,
                               const QDateTime& adj, int status)
{
    QString dateTimeFormat = QLocale().dateFormat(QLocale::ShortFormat);

    if (!dateTimeFormat.contains(QLatin1String("yyyy")))
    {
        dateTimeFormat.replace(QLatin1String("yy"),
                               QLatin1String("yyyy"));
    }

    dateTimeFormat.append(QLatin1String(" hh:mm:ss"));

    DItemsListViewItem* const item = listView()->findItem(url);

    if (item)
    {
        QStringList errors;

        if (status & META_TIME_ERROR)
        {
            errors << i18n("Failed to update metadata timestamp");
        }

        if (status & FILE_TIME_ERROR)
        {
            errors << i18n("Failed to update file timestamp");
        }

        if      (status & CLRSTATUS_ERROR)
        {
            item->setText(STATUS, QLatin1String(""));
        }
        else if (errors.isEmpty())
        {
            // Processed without error
            item->setText(STATUS, i18n("Success"));
        }
        else
        {
            item->setText(STATUS, errors.join(QLatin1String(" | ")));
        }

        if (org.isValid())
        {
            item->setText(TIMESTAMP_USED, org.toString(dateTimeFormat));
        }
        else
        {
            item->setText(TIMESTAMP_USED, i18n("not valid"));
        }

        if (adj.isValid())
        {
            item->setText(TIMESTAMP_UPDATED, adj.toString(dateTimeFormat));
        }
        else
        {
            item->setText(TIMESTAMP_UPDATED, i18n("not valid"));
        }
    }
}

void TimeAdjustList::setWaitStatus()
{
    QTreeWidgetItemIterator it(listView());

    while (*it)
    {
        DItemsListViewItem* const item = dynamic_cast<DItemsListViewItem*>(*it);

        if (item)
        {
            item->setText(STATUS,            i18n("Please wait ..."));
            item->setText(TIMESTAMP_USED,    QLatin1String(""));
            item->setText(TIMESTAMP_UPDATED, QLatin1String(""));
        }

        ++it;
    }
}

} // namespace DigikamGenericTimeAdjustPlugin
