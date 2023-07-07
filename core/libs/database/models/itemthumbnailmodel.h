/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries with support for thumbnail loading
 *
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText:      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_THUMBNAIL_MODEL_H
#define DIGIKAM_ITEM_THUMBNAIL_MODEL_H

// Local includes

#include "itemmodel.h"
#include "thumbnailsize.h"
#include "digikam_export.h"

namespace Digikam
{

class LoadingDescription;
class ThumbnailLoadThread;

class DIGIKAM_DATABASE_EXPORT ItemThumbnailModel : public ItemModel
{
    Q_OBJECT

public:

    /**
     *  An ItemModel that supports thumbnail loading.
     *  You need to set a ThumbnailLoadThread to enable thumbnail loading.
     *  Adjust the thumbnail size to your needs.
     *  Note that setKeepsFilePathCache is enabled per default.
     */
    explicit ItemThumbnailModel(QObject* const parent);
    ~ItemThumbnailModel()                                                             override;

    /**
     * Enable thumbnail loading and set the thread that shall be used.
     * The thumbnail size of this thread will be adjusted.
     */
    void setThumbnailLoadThread(ThumbnailLoadThread* const thread);
    ThumbnailLoadThread* thumbnailLoadThread()                                  const;

    /// Set the thumbnail size to use
    void setThumbnailSize(const ThumbnailSize& thumbSize);

    /// If you want to fix a size for preloading, do it here.
    void setPreloadThumbnailSize(const ThumbnailSize& thumbSize);

    void setExifRotate(bool rotate);

    /**
     * Enable emitting dataChanged() when a thumbnail becomes available.
     * The thumbnailAvailable() signal will be emitted in any case.
     * Default is true.
     */
    void setEmitDataChanged(bool emitSignal);

    /**
     * Enable preloading of thumbnails:
     * If preloading is enabled, for every entry in the model a thumbnail generation is started.
     * Default: false.
     */
    void setPreloadThumbnails(bool preload);

    ThumbnailSize thumbnailSize()                                               const;

    /**
     * Handles the ThumbnailRole.
     * If the pixmap is available, returns it in the QVariant.
     * If it still needs to be loaded, returns a null QVariant and emits
     * thumbnailAvailable() as soon as it is available.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole)         const override;

    /**
     * You can override the current thumbnail size by giving an integer value for ThumbnailRole.
     * Set a null QVariant to use the thumbnail size set by setThumbnailSize() again.
     * The index given here is ignored for this purpose.
     */
    bool setData(const QModelIndex& index,
                         const QVariant& value,
                         int role = Qt::DisplayRole)                                  override;

public Q_SLOTS:

    /**
     * Prepare the thumbnail loading for the given indexes
     */
    void prepareThumbnails(const QList<QModelIndex>& indexesToPrepare);
    void prepareThumbnails(const QList<QModelIndex>& indexesToPrepare,
                           const ThumbnailSize& thumbSize);

    /**
     * Preload thumbnail for the given infos resp. indexes.
     * Note: Use setPreloadThumbnails to automatically preload all entries in the model.
     * Note: This only ensures thumbnail generation. It is not guaranteed that pixmaps
     * are stored in the cache. For thumbnails that are expect to be drawn immediately,
     * include them in prepareThumbnails().
     * Note: Stops preloading of previously added thumbnails.
     */
    void preloadThumbnails(const QList<ItemInfo>&);
    void preloadThumbnails(const QList<QModelIndex>&);
    void preloadAllThumbnails();

Q_SIGNALS:

    void thumbnailAvailable(const QModelIndex& index, int requestedSize);
    void thumbnailFailed(const QModelIndex& index, int requestedSize);

protected:

    void imageInfosCleared()                                                  override;

protected Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription& loadingDescription,
                             const QPixmap& thumb);

private:

    // Disable
    ItemThumbnailModel(const ItemThumbnailModel&)            = delete;
    ItemThumbnailModel& operator=(const ItemThumbnailModel&) = delete;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_THUMBNAIL_MODEL_H
