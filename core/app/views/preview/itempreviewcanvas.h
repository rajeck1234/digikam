/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded item-view to show the canvas preview widget.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier  <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_PREVIEW_CANVAS_H
#define DIGIKAM_ITEM_PREVIEW_CANVAS_H

// Local includes

#include "digikam_config.h"
#include "dimgpreviewitem.h"
#include "iteminfo.h"

class QGraphicsSceneContextMenuEvent;
class QGraphicsSceneHoverEvent;

namespace Digikam
{

class FaceGroup;

class ItemPreviewCanvas : public DImgPreviewItem
{
    Q_OBJECT

public:

    explicit ItemPreviewCanvas();
    ~ItemPreviewCanvas()                                      override;

    void setItemInfo(const ItemInfo& info);
    ItemInfo imageInfo()                            const;

    void setFaceGroup(FaceGroup* const group);

protected:

    void hoverEnterEvent(QGraphicsSceneHoverEvent* e)         override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* e)         override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent* e)          override;

private:

    // Disable
    ItemPreviewCanvas(QObject*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_PREVIEW_CANVAS_H
