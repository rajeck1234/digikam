/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-20-07
 * Description : Thumbnail bar for import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMPORT_THUMBNAIL_BAR_H
#define DIGIKAM_IMPORT_THUMBNAIL_BAR_H

// Local includes

#include "importcategorizedview.h"

namespace Digikam
{

class ImportThumbnailBar : public ImportCategorizedView
{
    Q_OBJECT

public:

    explicit ImportThumbnailBar(QWidget* const parent = nullptr);
    ~ImportThumbnailBar() override;

    /**
     * This installs a duplicate filter model, if the ImportItemModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ImportItemModel* model, ImportSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex()                            const;
    QModelIndex lastIndex()                             const;

    /**
     * Sets the policy always for the one scroll bar which is relevant, depending on orientation
     */
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    void installOverlays();

public Q_SLOTS:

    void assignRating(const QList<QModelIndex>& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    void slotSetupChanged() override;
    bool event(QEvent*)     override;

private:

    // Disable
    ImportThumbnailBar(const ImportThumbnailBar&)            = delete;
    ImportThumbnailBar& operator=(const ImportThumbnailBar&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMPORT_THUMBNAIL_BAR_H
