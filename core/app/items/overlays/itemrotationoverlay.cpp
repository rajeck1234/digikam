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

#include "itemrotationoverlay.h"

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "itemcategorizedview.h"
#include "iteminfo.h"
#include "itemmodel.h"
#include "itemviewhoverbutton.h"

namespace Digikam
{

ItemRotateOverlayButton::ItemRotateOverlayButton(ItemRotateOverlayDirection dir, QAbstractItemView* const parentView)
    : ItemViewHoverButton(parentView),
      m_direction        (dir)
{
    setup();
}

QSize ItemRotateOverlayButton::sizeHint() const
{
    return QSize(32, 32);
}

QIcon ItemRotateOverlayButton::icon()
{
    if (m_direction == ItemRotateOverlayLeft)
    {
        return QIcon::fromTheme(QLatin1String("object-rotate-left"));
    }
    else
    {
        return QIcon::fromTheme(QLatin1String("object-rotate-right"));
    }
}

void ItemRotateOverlayButton::updateToolTip()
{
    if (m_direction == ItemRotateOverlayLeft)
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Left"));
    }
    else
    {
        setToolTip(i18nc("@info:tooltip", "Rotate Right"));
    }
}

// --------------------------------------------------------------------

ItemRotateOverlay::ItemRotateOverlay(ItemRotateOverlayDirection dir, QObject* const parent)
    : HoverButtonDelegateOverlay(parent),
      m_direction               (dir)
{
}

void ItemRotateOverlay::setActive(bool active)
{
    HoverButtonDelegateOverlay::setActive(active);

    if (active)
    {
        connect(button(), SIGNAL(clicked(bool)),
                this, SLOT(slotClicked()));
    }

    // if !active, button() is deleted
}

ItemViewHoverButton* ItemRotateOverlay::createButton()
{
    return new ItemRotateOverlayButton(m_direction, view());
}

void ItemRotateOverlay::updateButton(const QModelIndex& index)
{
    const QRect rect = m_view->visualRect(index);
    const int size   = qBound(16, rect.width() / 8 - 2, 48);
    const int gap    = 5;
    const int x      = rect.right() - 2*gap - (isLeft() ? size*3 + 2 : size*2 + 2);
    const int y      = rect.top() + gap;
    button()->resize(size, size);
    button()->move(QPoint(x, y));
}

void ItemRotateOverlay::slotClicked()
{
    QModelIndex index = button()->index();

    if (index.isValid())
    {
/*
        QItemSelectionModel* selModel = m_view->selectionModel();
        selModel->reset();
        selModel->select(index, QItemSelectionModel::Select);
        selModel->setCurrentIndex(index, QItemSelectionModel::Current);
*/
        Q_EMIT signalRotate(affectedIndexes(index));
    }
}

bool ItemRotateOverlay::checkIndex(const QModelIndex& index) const
{
    ItemInfo info = ItemModel::retrieveItemInfo(index);

    return ((info.category() == DatabaseItem::Image) ||
            (info.category() == DatabaseItem::Video));
}

void ItemRotateOverlay::widgetEnterEvent()
{
    widgetEnterNotifyMultiple(button()->index());
}

void ItemRotateOverlay::widgetLeaveEvent()
{
    widgetLeaveNotifyMultiple();
}

} // namespace Digikam
