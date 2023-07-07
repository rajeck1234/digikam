/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for items - the delegate (private container)
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_VIEW_DELEGATE_P_H
#define DIGIKAM_ITEM_VIEW_DELEGATE_P_H

#include "itemviewdelegate.h"

// Qt includes

#include <QCache>
#include <QFont>
#include <QPainter>
#include <QPolygon>

// Local includes

#include "digikam_debug.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemViewDelegate;

class DIGIKAM_EXPORT ItemViewDelegatePrivate
{
public:

    explicit ItemViewDelegatePrivate();
    virtual ~ItemViewDelegatePrivate() = default;

    void init(ItemViewDelegate* const _q);

    void makeStarPolygon();

    /**
     * Resets cached rects. Remember to reimplement in subclass for added rects.
     */
    virtual void clearRects();

public:

    int                       spacing;
    QSize                     gridSize;

    QRect                     rect;
    QRect                     ratingRect;

    QPixmap                   regPixmap;
    QPixmap                   selPixmap;
    QVector<QPixmap>          ratingPixmaps;

    QFont                     font;
    QFont                     fontReg;
    QFont                     fontCom;
    QFont                     fontXtra;

    QPolygon                  starPolygon;
    QSize                     starPolygonSize;

    ThumbnailSize             thumbSize;

    QPersistentModelIndex     editingRating;

    ItemViewDelegate*         q;

    QRect                     oneRowRegRect;
    QRect                     oneRowComRect;
    QRect                     oneRowXtraRect;

    /// constant values for drawing
    int                       radius;
    int                       margin;

private:

    Q_DISABLE_COPY(ItemViewDelegatePrivate)
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_VIEW_DELEGATE_P_H
