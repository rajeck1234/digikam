/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-31
 * Description : rotate icon view item at mouse hover
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_ROTATION_OVERLAY_H
#define DIGIKAM_ITEM_ROTATION_OVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"

namespace Digikam
{

enum ItemRotateOverlayDirection
{
    ItemRotateOverlayLeft,
    ItemRotateOverlayRight
};

class ItemRotateOverlayButton : public ItemViewHoverButton
{
    Q_OBJECT

public:

    ItemRotateOverlayButton(ItemRotateOverlayDirection dir, QAbstractItemView* const parentView);
    QSize sizeHint() const  override;

protected:

    QIcon icon()            override;
    void updateToolTip()    override;

protected:

    ItemRotateOverlayDirection const m_direction;
};

// --------------------------------------------------------------------

class ItemRotateOverlay : public HoverButtonDelegateOverlay
{
    Q_OBJECT

public:

    ItemRotateOverlay(ItemRotateOverlayDirection dir, QObject* const parent);
    void setActive(bool active)                     override;

    ItemRotateOverlayDirection direction() const { return m_direction; }
    bool isLeft() const  { return m_direction  == ItemRotateOverlayLeft; }
    bool isRight() const { return m_direction == ItemRotateOverlayRight; }

    static ItemRotateOverlay* left(QObject* const parent)  { return new ItemRotateOverlay(ItemRotateOverlayLeft, parent);  }
    static ItemRotateOverlay* right(QObject* const parent) { return new ItemRotateOverlay(ItemRotateOverlayRight, parent); }

Q_SIGNALS:

    void signalRotate(const QList<QModelIndex>& indexes);

protected:

    ItemViewHoverButton* createButton()             override;
    void updateButton(const QModelIndex& index)     override;
    bool checkIndex(const QModelIndex& index) const override;
    void widgetEnterEvent()                         override;
    void widgetLeaveEvent()                         override;

private Q_SLOTS:

    void slotClicked();

private:

    ItemRotateOverlayDirection const m_direction;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_ROTATION_OVERLAY_H
