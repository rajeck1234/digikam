/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-01-11
 * Description : shared image loading and caching
 *
 * SPDX-FileCopyrightText: 2005-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LOADING_CACHE_H
#define DIGIKAM_LOADING_CACHE_H

// Qt includes

#include <QPixmap>

// Local includes

#include "dimg.h"
#include "loadsavethread.h"
#include "digikam_export.h"

namespace Digikam
{

class ICCSettingsContainer;

class LoadingProcessListener
{
public:

    LoadingProcessListener();
    virtual ~LoadingProcessListener();
    virtual bool querySendNotifyEvent()                                                     const = 0;
    virtual void setResult(const LoadingDescription& loadingDescription, const DImg& img)         = 0;
    virtual LoadSaveNotifier* loadSaveNotifier()                                            const = 0;
    virtual LoadSaveThread::AccessMode accessMode()                                         const = 0;

private:

    // Disable
    LoadingProcessListener(const LoadingProcessListener&)            = delete;
    LoadingProcessListener& operator=(const LoadingProcessListener&) = delete;
};

// --------------------------------------------------------------------------------------------------------------

class LoadingProcess
{
public:

    LoadingProcess();
    virtual ~LoadingProcess();
    virtual bool completed()                                                                             const = 0;
    virtual QString cacheKey()                                                                           const = 0;
    virtual void addListener(LoadingProcessListener* const listener)                                           = 0;
    virtual void removeListener(LoadingProcessListener* const listener)                                        = 0;
    virtual void notifyNewLoadingProcess(LoadingProcess* const process, const LoadingDescription& description) = 0;

private:

    // Disable
    LoadingProcess(const LoadingProcess&)            = delete;
    LoadingProcess& operator=(const LoadingProcess&) = delete;
};

// --------------------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT LoadingCacheFileWatch : public QObject
{
    Q_OBJECT

public:

    LoadingCacheFileWatch();
    ~LoadingCacheFileWatch() override;

    void addedImage(const QString& filePath);
    void removeImage(const QString& filePath);
    void checkFileWatch(const QString& filePath);

protected:

    /**
     * Convenience method.
     * Call this to tell the cache to remove stored images for filePath from the cache.
     * Calling this method is fast, you do not need to check if the file is contained in the cache.
     * Do not hold the CacheLock when calling this method.
     */
    void notifyFileChanged(const QString& filePath);

protected:

    friend class LoadingCache;

    QHash<QString, QPair<qint64, QDateTime> > m_watchHash;
    class LoadingCache*                       m_cache;

private:

    // Disable
    LoadingCacheFileWatch(const LoadingCacheFileWatch&)            = delete;
    LoadingCacheFileWatch& operator=(const LoadingCacheFileWatch&) = delete;
    LoadingCacheFileWatch(QObject*)                                = delete;
};

// --------------------------------------------------------------------------------------------------------------

class DIGIKAM_EXPORT LoadingCache : public QObject
{
    Q_OBJECT

public:

    /**
     * NOTE: !! All methods of LoadingCache shall only be called when a CacheLock is held !!
     */

    class DIGIKAM_EXPORT CacheLock
    {
    public:

        explicit CacheLock(LoadingCache* const cache);
        ~CacheLock();
        void wakeAll();
        void timedWait();

    private:

        LoadingCache* m_cache;
    };

public:

    static LoadingCache* cache();
    static void cleanUp();

    /**
     * Retrieves an image for the given string from the cache,
     * or 0 if no image is found.
     */
    DImg* retrieveImage(const QString& cacheKey) const;

    /**
     * Returns whether the given DImg fits in the cache.
     */
    bool isCacheable(const DImg& img) const;

    /**
     * Put image into for given string into the cache.
     * Returns true if image has been put in the cache, false otherwise.
     * Ownership of the DImg instance is passed to the cache.
     * When it cannot be put in the cache it is deleted.
     * The third parameter specifies a file path that will be watched.
     * If this file changes, the object will be removed from the cache.
     */
    bool putImage(const QString& cacheKey, const DImg& img, const QString& filePath) const;

    /**
     * Remove entries for the given cacheKey from the cache
     */
    void removeImage(const QString& cacheKey);

    /**
     * Remove all entries from the cache
     */
    void removeImages();

    // ------- Loading process management -----------------------------------

    /**
     * Find the loading process for given cacheKey, or 0 if not found
     */
    LoadingProcess* retrieveLoadingProcess(const QString& cacheKey) const;

    /**
     * Add a loading process to the list. Only one loading process
     * for the same cache key is registered at a time.
     */
    void addLoadingProcess(LoadingProcess* const process);

    /**
     * Remove loading process for given cache key
     */
    void removeLoadingProcess(LoadingProcess* const process);

    /**
     * Notify all currently registered loading processes
     */
    void notifyNewLoadingProcess(LoadingProcess* const process, const LoadingDescription& description);

    /**
     * Sets the cache size in megabytes.
     * The thumbnail cache is not affected and setThumbnailCacheSize takes the maximum number.
     */
    void setCacheSize(int megabytes);

    // ------- Thumbnail cache -----------------------------------

    /**
     * The LoadingCache support both the caching of QImage and QPixmap objects.
     * QPixmaps can only be accessed from the main thread, so the tasks cannot access this cache.
     */

    /**
     * Retrieves a thumbnail for the given filePath from the thumbnail cache,
     * or a 0 if the thumbnail is not found.
     */
    const QImage* retrieveThumbnail(const QString& cacheKey) const;
    const QPixmap* retrieveThumbnailPixmap(const QString& cacheKey) const;
    bool  hasThumbnailPixmap(const QString& cacheKey) const;

    /**
     * Puts a thumbnail into the thumbnail cache.
     */
    void putThumbnail(const QString& cacheKey, const QImage& thumb, const QString& filePath);
    void putThumbnail(const QString& cacheKey, const QPixmap& thumb, const QString& filePath);

    /**
     * Remove the thumbnail for the given file path from the thumbnail cache
     */
    void removeThumbnail(const QString& cacheKey);

    /**
     * Remove all thumbnails
     */
    void removeThumbnails();

    /**
     * Sets the size of the thumbnail cache
     *  @param numberOfQImages  The maximum number of thumbnails of max possible size in QImage format
     *                          that will be cached. If the size of the images is smaller, a larger
     *                          number will be cached.
     *  @param numberOfQPixmaps The maximum number of thumbnails of max possible size in QPixmap format
     *                          that will be cached. If the size of the images is smaller, a larger
     *                          number will be cached.
     * Note: The main cache is unaffected by this method,
     *       and setCacheSize takes megabytes as parameter.
     * Note: A good caching strategy will be to set one of the numbers to 0
     * Default values: (0, 100)
     */
    void setThumbnailCacheSize(int numberOfQImages, int numberOfQPixmaps);

    // ------- File Watch Management -----------------------------------

    /**
     * Sets a LoadingCacheFileWatch to watch the files contained in this cache.
     * Ownership of this object is transferred to the cache.
     */
    void setFileWatch(LoadingCacheFileWatch* const watch);

    /**
     * Remove all entries from cache that were loaded from filePath.
     * Emits relevant signals if notify = true.
     */
    void notifyFileChanged(const QString& filePath, bool notify = true);

Q_SIGNALS:

    /**
     * This signal is emitted when the cache is notified that a file was changed.
     * There is no information in this signal if the file was ever contained in the cache.
     * The signal may be emitted under CacheLock. Strongly consider a queued connection.
     */
    void fileChanged(const QString& filePath);

private Q_SLOTS:

    void iccSettingsChanged(const ICCSettingsContainer& current, const ICCSettingsContainer& previous);

private:

    // Disabled
    LoadingCache();
    explicit LoadingCache(QObject*)              = delete;
    ~LoadingCache() override;

    LoadingCache(const LoadingCache&)            = delete;
    LoadingCache& operator=(const LoadingCache&) = delete;

private:

    friend class LoadingCacheFileWatch;
    friend class CacheLock;

private:

    static LoadingCache* m_instance;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_LOADING_CACHE_H
