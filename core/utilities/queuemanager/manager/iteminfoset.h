/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : queue pool info container.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_ITEM_INFO_SET_H
#define DIGIKAM_BQM_ITEM_INFO_SET_H

// Qt includes

#include <QList>

// Local includes

#include "iteminfo.h"

namespace Digikam
{

/**
 * A container of associated ItemInfo and queue id.
 */
class ItemInfoSet
{
public:

    ItemInfoSet()
        : queueId(0)
    {
    };

    ItemInfoSet(int id, const ItemInfo& inf)
        : queueId(id),
          info   (inf)
    {
    };

    ~ItemInfoSet()
    {
    };

    int      queueId;
    ItemInfo info;
};

/**
 * A list of all queued items from the pool.
 */
typedef QList<ItemInfoSet> QueuePoolItemsList;

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::ItemInfoSet)
Q_DECLARE_METATYPE(Digikam::QueuePoolItemsList)

#endif // DIGIKAM_BQM_ITEM_INFO_SET_H
