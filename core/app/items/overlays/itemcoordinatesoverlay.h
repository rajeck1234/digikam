/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-05-28
 * Description : overlay for GPS location indicator
 *
 * SPDX-FileCopyrightText: 2014-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_COORDINATES_OVERLAY_H
#define DIGIKAM_ITEM_COORDINATES_OVERLAY_H

// Qt includes

#include <QAbstractButton>
#include <QAbstractItemView>

// Local includes

#include "itemviewhoverbutton.h"
#include "itemdelegateoverlay.h"
#include "itemviewdelegate.h"

namespace Digikam
{

class CoordinatesOverlayWidget : public QAbstractButton
{
    Q_OBJECT

public:

    explicit CoordinatesOverlayWidget(QWidget* const parent = nullptr);

protected:

    void paintEvent(QPaintEvent*) override;
};

// ----------------------------------------------------------------------

class ItemCoordinatesOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewDelegate)

public:

    explicit ItemCoordinatesOverlay(QObject* const parent);
    CoordinatesOverlayWidget* buttonWidget() const;

protected:

    void updatePosition();

    QWidget* createWidget() override;
    void setActive(bool active) override;
    void visualChange() override;
    bool checkIndex(const QModelIndex& index) const override;
    void slotEntered(const QModelIndex& index) override;

protected:

    QPersistentModelIndex m_index;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_COORDINATES_OVERLAY_H
