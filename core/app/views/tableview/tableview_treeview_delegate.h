/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-03-02
 * Description : Table view: Tree view subelement
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TABLE_VIEW_TREEVIEW_DELEGATE_H
#define DIGIKAM_TABLE_VIEW_TREEVIEW_DELEGATE_H

// Qt includes

#include <QItemDelegate>
#include <QTreeView>

// Local includes

#include "digikam_export.h"
#include "itemalbummodel.h"
#include "itemfiltermodel.h"
#include "statesavingobject.h"
#include "tableview_columnfactory.h"
#include "tableview_shared.h"
#include "thumbnailloadthread.h"

class QMenu;
class QContextMenuEvent;

namespace Digikam
{

class TableViewItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    explicit TableViewItemDelegate(TableViewShared* const tableViewShared, QObject* const parent = nullptr);
    ~TableViewItemDelegate() override;

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& tableViewModelIndex)                const override;

private:

    TableViewShared* const s;
};

} // namespace Digikam

#endif // DIGIKAM_TABLE_VIEW_TREEVIEW_DELEGATE_H
