/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : header list view item
 *
 * SPDX-FileCopyrightText: 2010-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_HEADER_LIST_ITEM_H
#define DIGIKAM_DB_HEADER_LIST_ITEM_H

// Qt includes

#include <QObject>
#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace Digikam
{

class DbHeaderListItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT

public:

    explicit DbHeaderListItem(QTreeWidget* parent, const QString& key);
    ~DbHeaderListItem() override;

private Q_SLOTS:

    void slotThemeChanged();

private:

    DbHeaderListItem(const DbHeaderListItem&)            = delete;
    DbHeaderListItem& operator=(const DbHeaderListItem&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_DB_HEADER_LIST_ITEM_H
