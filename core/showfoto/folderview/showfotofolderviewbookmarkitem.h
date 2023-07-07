/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-08-27
 * Description : Showfoto folder view bookmarks item
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOWFOTO_FOLDER_VIEW_BOOKMARK_ITEM_H
#define SHOWFOTO_FOLDER_VIEW_BOOKMARK_ITEM_H

// Qt includes

#include <QString>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace ShowFoto
{

class ShowfotoFolderViewBookmarkItem : public QTreeWidgetItem
{

public:

    explicit ShowfotoFolderViewBookmarkItem(QTreeWidgetItem* const parent);
    ~ShowfotoFolderViewBookmarkItem()    override;

    void setPath(const QString&);
    QString path()      const;

private:

    QString m_path;

    Q_DISABLE_COPY(ShowfotoFolderViewBookmarkItem)
};

} // namespace ShowFoto

#endif // SHOWFOTO_FOLDER_VIEW_BOOKMARK_ITEM_H
