/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt model-view for items - the delegate
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DIGIKAMITEM_DELEGATE_P_H
#define DIGIKAM_DIGIKAMITEM_DELEGATE_P_H

// Qt includes

#include <QCache>

// Local includes

#include "itemdelegate_p.h"

namespace Digikam
{

class Q_DECL_HIDDEN DigikamItemDelegatePrivate : public ItemDelegate::ItemDelegatePrivate
{
public:

    explicit DigikamItemDelegatePrivate()
    {
    }

    ~DigikamItemDelegatePrivate() override;

    void init(DigikamItemDelegate* const q, ItemCategorizedView* const parent);
};

// -----------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ItemFaceDelegatePrivate : public DigikamItemDelegatePrivate
{
public:

    explicit ItemFaceDelegatePrivate()
    {
    }
};

} // namespace Digikam

#endif // DIGIKAM_DIGIKAMITEM_DELEGATE_P_H
