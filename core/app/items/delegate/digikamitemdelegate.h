/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIGIKAMITEM_DELEGATE_H
#define DIGIKAM_DIGIKAMITEM_DELEGATE_H

// Local includes

#include "itemdelegate.h"

namespace Digikam
{

class ItemCategoryDrawer;
class DigikamItemDelegatePrivate;

class DigikamItemDelegate : public ItemDelegate
{
    Q_OBJECT

public:

    explicit DigikamItemDelegate(ItemCategorizedView* parent);
    ~DigikamItemDelegate() override;

protected:

    void updateRects() override;

    DigikamItemDelegate(DigikamItemDelegatePrivate& dd, ItemCategorizedView* parent);

private:

    // Disable
    DigikamItemDelegate(QObject*) = delete;

private:

    Q_DECLARE_PRIVATE(DigikamItemDelegate)
};

} // namespace Digikam

#endif // DIGIKAM_DIGIKAMITEM_DELEGATE_H
