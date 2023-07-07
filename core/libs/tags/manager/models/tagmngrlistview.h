/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : Reimplemented QListView for Tags Manager, with support for
 *               drag-n-drop
 *
 * SPDX-FileCopyrightText: 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TAG_MNGR_LIST_VIEW_H
#define DIGIKAM_TAG_MNGR_LIST_VIEW_H

// Qt includes

#include <QTreeView>

class QPoint;

namespace Digikam
{

class TagMngrListView : public QTreeView
{
    Q_OBJECT

public:

    explicit TagMngrListView(QWidget* const parent = nullptr);

    QModelIndexList mySelectedIndexes();

protected:

    /**
     * Reimplemented methods to enable custom drag-n-drop in QListView
     */
    void startDrag(Qt::DropActions supportedActions)    override;
    void dropEvent(QDropEvent *e)                       override;

    QModelIndex indexVisuallyAt(const QPoint& p);

    /**
     * @brief contextMenuEvent - reimplemented method from QListView
     *                           to handle custom context menu
     */
    void contextMenuEvent(QContextMenuEvent* event)     override;


public Q_SLOTS:

    /**
     * @brief slotDeleteSelected - delete selected item from Quick Access List
     */
    void slotDeleteSelected();
};

} // namespace Digikam

#endif // DIGIKAM_TAG_MNGR_LIST_VIEW_H
