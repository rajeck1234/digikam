/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-11
 * Description : light table thumbs bar
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LIGHT_TABLE_THUMB_BAR_H
#define DIGIKAM_LIGHT_TABLE_THUMB_BAR_H

// Local includes

#include "itemthumbnailbar.h"
#include "iteminfo.h"
#include "itemlistmodel.h"

namespace Digikam
{

class LightTableThumbBar : public ItemThumbnailBar
{
    Q_OBJECT

public:

    explicit LightTableThumbBar(QWidget* const parent);
    ~LightTableThumbBar()                                                  override;

    void clear();

    void setItems(const ItemInfoList& list);
    void setOnLeftPanel(const ItemInfo& info);
    void setOnRightPanel(const ItemInfo& info);
    void setNavigateByPair(bool b);

    void removeItemByInfo(const ItemInfo& info);
    void toggleTag(int tagID);

    bool isOnLeftPanel(const ItemInfo& info)             const;
    bool isOnRightPanel(const ItemInfo& info)            const;
    int  countItems()                                    const;
    QModelIndex findItemByInfo(const ItemInfo& info)     const;
    ItemInfo   findItemByIndex(const QModelIndex& index) const;

Q_SIGNALS:

    void signalSetItemOnLeftPanel(const ItemInfo&);
    void signalSetItemOnRightPanel(const ItemInfo&);
    void signalEditItem(const ItemInfo&);
    void signalRemoveItem(const ItemInfo&);
    void signalDroppedItems(const QList<ItemInfo>&);
    void signalClearAll();
    void signalContentChanged();

public Q_SLOTS:

    void slotAssignPickLabel(int);
    void slotAssignColorLabel(int);
    void slotAssignRating(int);
    void slotRatingChanged(const QUrl&, int);
    void slotColorLabelChanged(const QUrl&, int);
    void slotPickLabelChanged(const QUrl&, int);
    void slotToggleTag(const QUrl&, int);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

private:

    void paintEvent(QPaintEvent*)                                          override;
    void showContextMenuOnInfo(QContextMenuEvent* e, const ItemInfo& info) override;
    void assignPickLabel(const ItemInfo& info, int pickId);
    void assignColorLabel(const ItemInfo& info, int colorId);
    void assignRating(const ItemInfo& info, int rating);
    void toggleTag(const ItemInfo& info, int tagID);

private Q_SLOTS:

    void slotSetupChanged()                                                override;

private:

    // Disable
    LightTableThumbBar(const LightTableThumbBar&)            = delete;
    LightTableThumbBar& operator=(const LightTableThumbBar&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_LIGHT_TABLE_THUMB_BAR_H
