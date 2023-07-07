/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-04-18
 * Description : ExifTool metadata list view item.
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EXIF_TOOL_LIST_VIEW_ITEM_H
#define DIGIKAM_EXIF_TOOL_LIST_VIEW_ITEM_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QString>

namespace Digikam
{

class ExifToolListViewGroup;

class ExifToolListViewItem : public QTreeWidgetItem
{
public:

    ExifToolListViewItem(ExifToolListViewGroup* const parent,
                         const QString& key,
                         const QString& value,
                         const QString& desc);
    ExifToolListViewItem(ExifToolListViewGroup* const parent,
                         const QString& key);

    ~ExifToolListViewItem() override;

    QString getKey()         const;
    QString getTitle()       const;
    QString getValue()       const;
    QString getDescription() const;

private:

    Q_DISABLE_COPY(ExifToolListViewItem)

private:

    class Private;
    Private* const d;

};

} // namespace Digikam

#endif // DIGIKAM_EXIF_TOOL_LIST_VIEW_ITEM_H
