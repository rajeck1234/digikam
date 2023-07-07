/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-09-27
 * Description : Showfoto stack view item
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOWFOTO_STACK_VIEW_ITEM_H
#define SHOWFOTO_STACK_VIEW_ITEM_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QTreeWidgetItem>

// Local includes

#include "showfotoiteminfo.h"

namespace ShowFoto
{

class ShowfotoStackViewList;

class ShowfotoStackViewItem : public QTreeWidgetItem
{

public:

    explicit ShowfotoStackViewItem(ShowfotoStackViewList* const parent);
    ~ShowfotoStackViewItem()                           override;

    void setInfo(const ShowfotoItemInfo&);
    ShowfotoItemInfo info()                      const;

    void setThumbnail(const QPixmap&);

    bool operator<(const QTreeWidgetItem& other) const override;

private:

    ShowfotoItemInfo m_info;

    Q_DISABLE_COPY(ShowfotoStackViewItem)
};

} // namespace ShowFoto

#endif // SHOWFOTO_STACK_VIEW_ITEM_H
