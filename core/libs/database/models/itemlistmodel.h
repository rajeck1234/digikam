/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-12-06
 * Description : An item model based on a static list
 *
 * SPDX-FileCopyrightText: 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_LIST_MODEL_H
#define DIGIKAM_ITEM_LIST_MODEL_H

// Local includes

#include "itemthumbnailmodel.h"
#include "digikam_export.h"

namespace Digikam
{

class ImageChangeset;
class CollectionImageChangeset;

class DIGIKAM_DATABASE_EXPORT ItemListModel : public ItemThumbnailModel
{
    Q_OBJECT

public:

    explicit ItemListModel(QObject* const parent = nullptr);
    ~ItemListModel() override;

    // NOTE: necessary methods to add and remove ItemInfos to the model are inherited from ItemModel

Q_SIGNALS:

    /**
     * Emitted when images are removed from the model because they are removed in the database
     */
    void imageInfosRemoved(const QList<ItemInfo>& infos);

protected Q_SLOTS:

    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_LIST_MODEL_H
