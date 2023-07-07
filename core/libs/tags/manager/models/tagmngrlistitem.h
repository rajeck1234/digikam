/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 20013-08-22
 * Description : List View Item for List View Model
 *
 * SPDX-FileCopyrightText: 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TAG_MNGR_LIST_ITEM_H
#define DIGIKAM_TAG_MNGR_LIST_ITEM_H

// Qt includes

#include <QList>
#include <QVariant>

namespace Digikam
{

class ListItem : public QObject
{
    Q_OBJECT

public:

    explicit ListItem(QList<QVariant>& data, ListItem* const parent = nullptr);
    ~ListItem() override;

    void appendChild(ListItem* const child);
    void removeTagId(int tagId);
    void deleteChild(int row);
    void setData(const QList<QVariant>& data);
    void removeAll();
    void appendList(const QList<ListItem*>& items);
    void deleteChild(ListItem* const item);

    ListItem* child(int row)                     const;
    QVariant data(int column)                    const;
    ListItem* parent()                           const;
    int childCount()                             const;
    int columnCount()                            const;
    int row()                                    const;
    QList<int> getTagIds()                       const;
    QList<ListItem*> allChildren()               const;
    bool equal(ListItem* const item)             const;

    /**
     * @brief containsItem  - search child items if contains a ListItem with
     *                        the same data as item
     * @param item          - ListItem pointer for which we should search if there
     *                        is a similar item
     * @return              - NULL if no similar item was found and a valid ListItem
     *                        if a ListItem with the same data was found
     */
    ListItem* containsItem(ListItem* const item) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_TAG_MNGR_LIST_ITEM_H
