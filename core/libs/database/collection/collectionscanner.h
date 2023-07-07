/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_COLLECTION_SCANNER_H
#define DIGIKAM_COLLECTION_SCANNER_H

// Qt includes

#include <QString>
#include <QObject>
#include <QList>

// Local includes

#include "iteminfo.h"
#include "digikam_export.h"
#include "coredbaccess.h"
#include "coredbalbuminfo.h"
#include "collectionscannerhints.h"

class QFileInfo;

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CollectionScanner : public QObject
{
    Q_OBJECT

public:

    enum FileScanMode
    {
        /**
         * The file will be scanned like it is done for any usual scan.
         * If it was not modified, no further action is taken.
         * If the file is not known yet, it will be fully scanned, or,
         * if an identical file is found, this data will be copied.
         */
        NormalScan,

        /**
         * The file will scanned like a modified file. Only a selected portion
         * of the metadata will be updated into the database.
         * If the file is not known yet, it will be fully scanned, or,
         * if an identical file is found, this data will be copied.
         */
        ModifiedScan,

        /**
         * The file will be scanned like a completely new file.
         * The complete metadata is re-read into the database.
         * No search for identical files will be done.
         */
        Rescan,

        /**
         * This is the same as Rescan but the database metadata
         * will be cleaned up if the corresponding metadata
         * write option is enabled.
         */
        CleanScan
    };

public:

    explicit CollectionScanner();
    ~CollectionScanner() override;

    //@{
    /**
     * Hints give the scanner additional info about things that happened in the past
     * carried out by higher level which the collection scanner cannot know.
     * They allow to carry out optimizations.
     * Record hints in a container, and provide the container to the collection scanner.
     * The Container set in setHintContainer must be one created by createContainer.
     */
    static CollectionScannerHintContainer* createHintContainer();
    void setHintContainer(CollectionScannerHintContainer* const container);
    void setUpdateHashHint(bool hint = true);
    //@}

    /**
     * Call this to enable the progress info signals.
     * Default is off.
     */
    void setSignalsEnabled(bool on);

    /**
     * Call this to enable emitting the total files to scan
     * (for progress info) before a complete collection scan.
     * Default is off. If on, setSignalEnabled() must be on to take effect.
     */
    void setNeedFileCount(bool on);

    /**
     * Call this to disable fast scan with album date check.
     * Default is on.
     */
    void setPerformFastScan(bool on);

    /**
     * Set an observer to be able to cancel a running scan
     */
    void setObserver(CollectionScannerObserver* const observer);

    void setDeferredFileScanning(bool defer);
    QStringList deferredAlbumPaths() const;

    /**
     * Returns item ids from new detected items
     */
    QList<qlonglong> getNewIdsList() const;

    // -----------------------------------------------------------------------------

    /** @name Scan operations
     */

    //@{

public:

    /**
     * Carries out a full scan on all available parts of the collection.
     * Only a full scan can finally remove deleted files from the database,
     * only a full scan will mark the database as scanned.
     * The database will be locked while running (Note: this is not done for partialScans).
     */
    void completeScan();

    /**
     * If you enable deferred file scanning for a completeScan(), new files
     * will not be scanned. The relevant albums are available from
     * deferredAlbumPaths() when completeScan() has finished.
     * You need to call finishCompleteScan() afterwards with the list
     * to get the same complete scan than undeferred completeScan().
     */
    void finishCompleteScan(const QStringList& albumPaths);

    /**
     * Returns if the initial scan of the database has been done.
     * This is the first complete scan after creation of a new database file
     * (or update requiring a rescan)
     */
    static bool databaseInitialScanDone();

    /**
     * Carries out a partial scan on the specified path of the collection.
     * The includes scanning for new files + albums and updating modified file data.
     * Files no longer found in the specified path however are not completely
     * removed, but only marked as removed. They will be removed only after a complete scan.
     */
    void partialScan(const QString& filePath);

    /**
     * Same procedure as above, but albumRoot and album is provided.
     */
    void partialScan(const QString& albumRoot, const QString& album);

    /**
     * The given file will be scanned according to the given mode.
     * Returns the image id of the file.
     */
    qlonglong scanFile(const QString& filePath, FileScanMode mode = ModifiedScan);

    /**
     * Same procedure as above, but albumRoot and album is provided.
     * If you already have this info it need not be retrieved.
     * Returns the image id of the file, or -1 on failure.
     */
    qlonglong scanFile(const QString& albumRoot,
                       const QString& album,
                       const QString& fileName,
                       FileScanMode mode = ModifiedScan);

    /**
     * The given file represented by the ItemInfo will be scanned according to mode
     */
    void scanFile(const ItemInfo& info, FileScanMode mode = ModifiedScan);

protected:

    void scanForStaleAlbums(const QList<CollectionLocation>& locations);
    void scanForStaleAlbums(const QList<int>& locationIdsToScan);
    void scanAlbumRoot(const CollectionLocation& location);
    void scanAlbum(const CollectionLocation& location, const QString& album, bool checkDate = false);
    void scanExistingFile(const QFileInfo& fi, qlonglong id);
    void scanFileNormal(const QFileInfo& info, const ItemScanInfo& scanInfo, bool checkSidecar = true);
    void scanModifiedFile(const QFileInfo& info, const ItemScanInfo& scanInfo);
    void scanFileUpdateHashReuseThumbnail(const QFileInfo& fi, const ItemScanInfo& scanInfo, bool fileWasEdited);
    void cleanScanFile(const QFileInfo& info, const ItemScanInfo& scanInfo);
    void rescanFile(const QFileInfo& info, const ItemScanInfo& scanInfo);
    void completeScanCleanupPart();
    void completeHistoryScanning();
    void finishHistoryScanning();
    void historyScanningStage2(const QList<qlonglong>& ids);
    void historyScanningStage3(const QList<qlonglong>& ids);

    qlonglong scanFile(const QFileInfo& fi, int albumId, qlonglong id, FileScanMode mode);
    qlonglong scanNewFile(const QFileInfo& info, int albumId);
    qlonglong scanNewFileFullScan(const QFileInfo& info, int albumId);

    //@}

    // -----------------------------------------------------------------------------

    /** @name Scan utilities
     */

    //@{

public:

    /**
     * Prepare the given albums to be removed,
     * typically by setting the albums as orphan
     * and removing all entries from the albums
     */
    void safelyRemoveAlbums(const QList<int>& albumIds);

    /**
     * When a file is derived from another file, typically through editing,
     * copy all relevant attributes from source file to the new file.
     */
    static void copyFileProperties(const ItemInfo& source, const ItemInfo& dest);

protected:

    void markDatabaseAsScanned();
    void mainEntryPoint(bool complete);
    int  checkAlbum(const CollectionLocation& location, const QString& album);
    void itemsWereRemoved(const QList<qlonglong>& removedIds);
    void updateRemovedItemsTime();
    void incrementDeleteRemovedCompleteScanCount();
    void resetDeleteRemovedSettings();
    bool checkDeleteRemoved();
    void loadNameFilters();
    int  countItemsInFolder(const QString& path);
    DatabaseItem::Category category(const QFileInfo& info);

    //@}

Q_SIGNALS:

    /**
     * Emitted once in scanAlbums(), the scan() methods, and updateItemsWithoutDate().
     * Gives the number of the files that need to be scanned.
     */
    void totalFilesToScan(int count);

    //@{
    /**
     * Notifies the begin of the scanning of the specified album root,
     * album, of stale files, or of the whole collection (after stale files)
     */
    void startScanningAlbumRoot(const QString& albumRoot);
    void startScanningAlbum(const QString& albumRoot, const QString& album);
    void startScanningForStaleAlbums();
    void startScanningAlbumRoots();
    void startCompleteScan();
    void signalScannedNewImage(const QFileInfo& info);
    //@}

    //@{
    /**
     * Emitted when the scanning has finished.
     * Note that start/finishScanningAlbum may be emitted recursively.
     */
    void finishedScanningAlbumRoot(const QString& albumRoot);
    void finishedScanningAlbum(const QString& albumRoot, const QString& album, int filesScanned);
    void finishedScanningForStaleAlbums();
    void finishedCompleteScan();
    //@}

    /**
     * Emitted between startScanningAlbum and finishedScanningAlbum.
     * In between these two signals, the sum of filesScanned of all sent signals
     * equals the one reported by finishedScanningAlbum()
     */
    void scannedFiles(int filesScanned);

    /**
     * Emitted when the observer told to cancel the scan
     */
    void cancelled();

private:

    // Disable
    CollectionScanner(QObject*) = delete;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_COLLECTION_SCANNER_H
