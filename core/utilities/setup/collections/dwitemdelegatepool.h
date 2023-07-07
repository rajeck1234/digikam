/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-15
 * Description : widget item delegate for setup collection view
 *
 * SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Rafael Fernández López <ereslibre at kde dot org>
 * SPDX-FileCopyrightText: 2008      by Kevin Ottens <ervin at kde dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DW_ITEM_DELEGATE_POOL_H
#define DIGIKAM_DW_ITEM_DELEGATE_POOL_H

// Qt includes

#include <QModelIndex>
#include <QHash>
#include <QList>

// Local includes

#include "digikam_export.h"

class QWidget;
class QStyleOptionViewItem;

namespace Digikam
{

class DWItemDelegate;
class DWItemDelegatePoolPrivate;

class DIGIKAM_GUI_EXPORT DWItemDelegatePool
{
public:

    enum UpdateWidgetsEnum
    {
        UpdateWidgets = 0,
        NotUpdateWidgets
    };

public:

    /**
     * Creates a new ItemDelegatePool.
     *
     * @param delegate the ItemDelegate for this pool.
     */
    explicit DWItemDelegatePool(DWItemDelegate* const delegate);
    ~DWItemDelegatePool();

    /**
     * @brief Returns the widget associated to @p index and @p widget
     * @param index The index to search into.
     * @param option a QStyleOptionViewItem.
     * @return A QList of the pointers to the widgets found.
     * @internal
     */
    QList<QWidget*> findWidgets(const QPersistentModelIndex& index, const QStyleOptionViewItem& option,
                                UpdateWidgetsEnum updateWidgets = UpdateWidgets) const;

    QList<QWidget*> invalidIndexesWidgets() const;

    void fullClear();

private:

    // Disable
    DWItemDelegatePool(const DWItemDelegatePool&)            = delete;
    DWItemDelegatePool& operator=(const DWItemDelegatePool&) = delete;

private:

    friend class DWItemDelegate;
    friend class DWItemDelegatePrivate;

    DWItemDelegatePoolPrivate* const d;
};

// -----------------------------------------------------------------------------------------------------------

class DWItemDelegateEventListener;

class DWItemDelegatePoolPrivate
{
public:

    explicit DWItemDelegatePoolPrivate(DWItemDelegate* const d);

public:

    DWItemDelegate*                                delegate;
    DWItemDelegateEventListener*                   eventListener;

    QList<QList<QWidget*> >                        allocatedWidgets;
    QHash<QPersistentModelIndex, QList<QWidget*> > usedWidgets;
    QHash<QWidget*, QPersistentModelIndex>         widgetInIndex;

    bool                                           clearing;

private:

    // Disable
    DWItemDelegatePoolPrivate(const DWItemDelegatePoolPrivate&)            = delete;
    DWItemDelegatePoolPrivate& operator=(const DWItemDelegatePoolPrivate&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_DW_ITEM_DELEGATE_POOL_H
