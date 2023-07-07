/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 02-08-2013
 * Description : Thumbnail bar for Showfoto
 *
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOW_FOTO_THUMB_NAIL_BAR_H
#define SHOW_FOTO_THUMB_NAIL_BAR_H

// Local Includes

#include "showfotocategorizedview.h"

namespace ShowFoto
{

class ShowfotoItemViewToolTip;

class ShowfotoThumbnailBar : public ShowfotoCategorizedView
{
    Q_OBJECT

public:

    explicit ShowfotoThumbnailBar(QWidget* const parent = nullptr);
    ~ShowfotoThumbnailBar()                                   override;

    /**
     * This installs a duplicate filter model, if the ShwofotoItemModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ShowfotoItemModel* model, ShowfotoSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex()                            const;
    QModelIndex lastIndex()                             const;

    int thumbnailIndexForUrl(const QUrl& url)           const;

    /**
     * Sets the policy always for the one scroll bar which is relevant, depending on orientation.
     */
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    ShowfotoItemInfo findItemByUrl(const QUrl& url);

    void installOverlays();

public Q_SLOTS:

    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    void slotSetupChanged()                                   override;
    bool event(QEvent*)                                       override;

private:

    // Disable
    ShowfotoThumbnailBar(const ShowfotoThumbnailBar&)            = delete;
    ShowfotoThumbnailBar& operator=(const ShowfotoThumbnailBar&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOW_FOTO_THUMB_NAIL_BAR_H
