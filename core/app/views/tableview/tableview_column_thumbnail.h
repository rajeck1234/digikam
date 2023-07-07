/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-02-25
 * Description : Table view column helpers: Thumbnail column
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2013      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TABLE_VIEW_COLUMN_THUMBNAIL_H
#define DIGIKAM_TABLE_VIEW_COLUMN_THUMBNAIL_H

// Local includes

#include "tableview_columnfactory.h"

namespace Digikam
{

class LoadingDescription;

namespace TableViewColumns
{

class ColumnThumbnail : public TableViewColumn
{
    Q_OBJECT

private:

    explicit ColumnThumbnail(TableViewShared* const tableViewShared,
                             const TableViewColumnConfiguration& pConfiguration,
                             QObject* const parent = nullptr);
    ~ColumnThumbnail() override;

public:

    ColumnFlags getColumnFlags()                                                                              const override;
    QString getTitle()                                                                                        const override;
    QVariant data(TableViewModel::Item* const item, const int role)                                           const override;
    bool paint(QPainter* const painter, const QStyleOptionViewItem& option, TableViewModel::Item* const item) const override;
    QSize sizeHint(const QStyleOptionViewItem& option, TableViewModel::Item* const item)                      const override;
    void updateThumbnailSize() override;

    static bool CreateFromConfiguration(TableViewShared* const tableViewShared,
                                        const TableViewColumnConfiguration& pConfiguration,
                                        TableViewColumn** const pNewColumn,
                                        QObject* const parent = nullptr);

    static TableViewColumnDescription getDescription();

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription, const QPixmap& thumb);

private:

    int m_thumbnailSize;
};

} // namespace TableViewColumns

} // namespace Digikam

#endif // DIGIKAM_TABLE_VIEW_COLUMN_THUMBNAIL_H
