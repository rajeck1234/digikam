/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded item-view to show the image preview widget.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itempreviewcanvas.h"

// Qt includes

#include <QGraphicsSceneContextMenuEvent>

// Local includes

#include "itempreviewview.h"
#include "facegroup.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemPreviewCanvas::Private
{
public:

    explicit Private()
      : group(nullptr)
    {
    }

    FaceGroup* group;
    ItemInfo   info;
};

ItemPreviewCanvas::ItemPreviewCanvas()
    : d(new Private)
{
    setAcceptHoverEvents(true);
}

ItemPreviewCanvas::~ItemPreviewCanvas()
{
    delete d;
}

void ItemPreviewCanvas::setFaceGroup(FaceGroup* const group)
{
    d->group = group;
}

void ItemPreviewCanvas::setItemInfo(const ItemInfo& info)
{
    d->info = info;
    setPath(info.filePath());
}

ItemInfo ItemPreviewCanvas::imageInfo() const
{
    return d->info;
}

void ItemPreviewCanvas::hoverEnterEvent(QGraphicsSceneHoverEvent* e)
{
    d->group->itemHoverEnterEvent(e);
}

void ItemPreviewCanvas::hoverLeaveEvent(QGraphicsSceneHoverEvent* e)
{
    d->group->itemHoverLeaveEvent(e);
}

void ItemPreviewCanvas::hoverMoveEvent(QGraphicsSceneHoverEvent* e)
{
    d->group->itemHoverMoveEvent(e);
}

} // namespace Digikam
