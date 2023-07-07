/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-02-21
 * Description : a generic list view item widget to
 *               display metadata
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_META_DATA_LIST_VIEW_ITEM_H
#define DIGIKAM_META_DATA_LIST_VIEW_ITEM_H

// Qt includes

#include <QString>
#include <QTreeWidgetItem>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetadataListViewItem : public QTreeWidgetItem
{
public:

    MetadataListViewItem(QTreeWidgetItem* const parent,
                         const QString& key,
                         const QString& title,
                         const QString& value);
    MetadataListViewItem(QTreeWidgetItem* const parent,
                         const QString& key,
                         const QString& title);
    ~MetadataListViewItem() override;

    QString getKey()   const;
    QString getTitle() const;
    QString getValue() const;

private:

    QString m_key;

private:

    Q_DISABLE_COPY(MetadataListViewItem)
};

} // namespace Digikam

#endif // DIGIKAM_META_DATA_LIST_VIEW_ITEM_H
