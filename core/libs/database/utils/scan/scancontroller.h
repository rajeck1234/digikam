/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-10-28
 * Description : scan item controller.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SCAN_CONTROLLER_H
#define DIGIKAM_SCAN_CONTROLLER_H

// Qt includes

#include <QThread>
#include <QString>
#include <QList>

// Local includes

#include "digikam_export.h"
#include "collectionscannerobserver.h"
#include "collectionscannerhints.h"
#include "collectionscanner.h"
#include "iteminfo.h"
#include "loadingcache.h"
#include "coredbchangesets.h"

namespace Digikam
{

class CollectionScanner;
class PAlbum;

class DIGIKAM_GUI_EXPORT ScanController : public QThread,
                                          public InitializationObserver
{
    Q_OBJECT

public:

    enum Advice
    {
        Success,
        ContinueWithoutDatabase,
        AbortImmediately
    };

public:

    /**
     * When writing metadata to the file, the file content on disk changes,
     * but the information is taken from the database; therefore,
     * the resulting scanning process can be optimized.
     *
     * Thus, if you write metadata of an ItemInfo from the database to disk,
     * do this in the scope of a FileMetadataWrite object.
     */
    class FileMetadataWrite
    {
    public:

        explicit FileMetadataWrite(const ItemInfo& info);
        ~FileMetadataWrite();

        void changed(bool wasChanged);

    protected:

        ItemInfo m_info;
        bool     m_changed;
    };

public:

    static ScanController* instance();

    /**
     * Carries out a complete collection scan, at the same time updating
     * the unique hash in the database and thumbnail database.
     * Synchronous, returns when ready.
     * The database will be locked while the scan is running.
     */
    void updateUniqueHash();

    void allowToScanDeferredFiles();

    /**
     * If necessary (modified or newly created, scans the file directly
     * Returns the up-to-date ItemInfo.
     */
    ItemInfo scannedInfo(const QString& filePath,
                         CollectionScanner::FileScanMode mode = CollectionScanner::NormalScan);

    /**
     * Returns item ids from new detected items
     */
    QList<qlonglong> getNewIdsList() const;

protected:

    void run() override;

private:

    explicit ScanController();
    ~ScanController() override;

    void setInitializationMessage();
    void createProgressDialog();

    bool continueQuery()                                            override;
    void connectCollectionScanner(CollectionScanner* const scanner) override;

    // -----------------------------------------------------------------------------

    /** @name Start Operations
     */

    //@{

public:

    /**
     * Restart thread after shutdown.
     */
    void restart();

    /**
     * Calls CoreDbAccess::checkReadyForUse(), providing progress
     * feedback if schema updating occurs.
     * Synchronous, returns when ready.
     */
    Advice databaseInitialization();

    /**
     * Carries out a complete collection scan, providing progress feedback.
     * Synchronous, returns when ready.
     * The database will be locked while the scan is running.
     * With the DeferFiles variant, deep files scanning (new files), the part
     * which can take long, will be done during the time after the method returns,
     * shortening the synchronous wait. After completeCollectionScanDeferFiles, you
     * need to call allowToScanDeferredFiles() once to enable scanning the deferred files.
     */
    void completeCollectionScan(bool defer = false);
    void completeCollectionScanDeferFiles();

    /**
     * Scan Whole collection without to display a progress dialog
     * or to manage splashscreen, as for NewItemsFinder tool.
     */
    void completeCollectionScanInBackground(bool defer, bool fastScan = true);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     */
    void scheduleCollectionScan(const QString& path);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     * A small delay may be introduced before the actual scanning starts,
     * so that you can call this often without checking for duplicates.
     * This method must only be used from the main thread.
     */
    void scheduleCollectionScanRelaxed(const QString& path);

    /**
     * Schedules a scan of the specified part of the collection.
     * Asynchronous, returns immediately.
     * A very long delay with timer restart may be introduced
     * before the actual scanning starts, so that you can call
     * this often without checking for duplicates.
     * This method is only for the QFileSystemWatcher.
     */
    void scheduleCollectionScanExternal(const QString& path);

    /**
     * Implementation of FileMetadataWrite, see there. Calling these methods is equivalent.
     */
    void beginFileMetadataWrite(const ItemInfo& info);

    /**
     * Resume a suspended collection scanning.
     * All scheduled scanning tasks are queued
     * and will be done when resumeCollectionScan()
     * has been called.
     * Calling these methods is recursive, you must resume
     * as often as you called suspend.
     */
    void resumeCollectionScan();

Q_SIGNALS:

    void databaseInitialized(bool success);
    void collectionScanStarted(const QString& message);

private Q_SLOTS:

    void slotStartCompleteScan();
    void slotStartScanningAlbum(const QString& albumRoot, const QString& album);
    void slotStartScanningAlbumRoot(const QString& albumRoot);
    void slotStartScanningForStaleAlbums();
    void slotStartScanningAlbumRoots();

private:

    /**
     * The file pointed to by file path will be scanned.
     * The scan is finished when returning from the method.
     */
    void scanFileDirectly(const QString& filePath);
    void scanFileDirectlyNormal(const ItemInfo& info);
    void completeCollectionScanCore(bool needTotalFiles, bool defer, bool fastScan);

    //@}

    // -----------------------------------------------------------------------------

    /** @name Progress Operations
     */

    //@{

public:

    /**
     * Hint at the imminent copy, move or rename of an album, so that the
     * collection scanner is informed about this.
     * If the album is renamed, give the new name in newAlbumName.
     * DstAlbum is the new parent album /
     * dstPath is the new parent directory of the album, so
     * do not include the album name to dstPath.
     */
    void hintAtMoveOrCopyOfAlbum(const PAlbum* const album,
                                 const PAlbum* const dstAlbum,
                                 const QString& newAlbumName = QString());
    void hintAtMoveOrCopyOfAlbum(const PAlbum* const album,
                                 const QString& dstPath,
                                 const QString& newAlbumName = QString());

    /**
     * Hint at the imminent copy, move or rename of items, so that the
     * collection scanner is informed about this.
     * Give the list of existing items, specify the destination with dstAlbum,
     * and give the names at destination in itemNames (Unless for rename, names wont usually change.
     * Give them nevertheless.)
     */
    void hintAtMoveOrCopyOfItems(const QList<qlonglong>& ids,
                                 const PAlbum* const dstAlbum,
                                 const QStringList& itemNames);
    void hintAtMoveOrCopyOfItem(qlonglong id,
                                const PAlbum* const dstAlbum,
                                const QString& itemName);

    /**
     * Hint at the fact that an item may have changed, although its modification date may not have changed.
     * Note that a scan of the containing directory will need to be triggered nonetheless for the hints to take effect.
     */
    void hintAtModificationOfItems(const QList<qlonglong>& ids);
    void hintAtModificationOfItem(qlonglong id);

Q_SIGNALS:

    void totalFilesToScan(int);
    void filesScanned(int);
    void scanningProgress(float progress);
    void triggerShowProgressDialog();
    void incrementProgressDialog(int);
    void progressFromInitialization(const QString&, int);

private Q_SLOTS:

    void slotTotalFilesToScan(int count);
    void slotScannedFiles(int scanned);
    void slotShowProgressDialog();
    void slotTriggerShowProgressDialog();
    void slotProgressFromInitialization(const QString& message, int numberOfSteps);
    void slotErrorFromInitialization(const QString& errorMessage);

private:

    void moreSchemaUpdateSteps(int numberOfSteps)                        override;
    void schemaUpdateProgress(const QString& message, int numberOfSteps) override;
    void error(const QString& errorMessage) override;

    AlbumCopyMoveHint hintForAlbum(const PAlbum* const album,
                                   int dstAlbumRootId,
                                   const QString& relativeDstPath,
                                   const QString& albumName);

    QList<AlbumCopyMoveHint> hintsForAlbum(const PAlbum* const album,
                                           int dstAlbumRootId,
                                           const QString& relativeDstPath,
                                           const QString& albumName);
    //@}

    // -----------------------------------------------------------------------------

    /** @name Stop Operations
     */

    //@{

public:

    /**
     * Wait for the thread to finish. Returns after all tasks are done.
     */
    void shutDown();

    /**
     * If the controller is currently processing a database update
     * (typically after first run),
     * cancel this hard and as soon as possible. Any progress may be lost.
     */
    void abortInitialization();

    /**
     * If the controller is currently doing a complete scan
     * (typically at startup), stop this operation.
     * It can be resumed later.
     */
    void cancelCompleteScan();

    /**
     * Cancels all running or scheduled operations and suspends scanning.
     * This method returns when all scanning has stopped.
     * This includes a call to suspendCollectionScan().
     * Restart with resumeCollectionScan.
     */
    void cancelAllAndSuspendCollectionScan();

    /**
     * Temporarily suspend collection scanning.
     * All scheduled scanning tasks are queued
     * and will be done when resumeCollectionScan()
     * has been called.
     * Calling these methods is recursive, you must resume
     * as often as you called suspend.
     */
    void suspendCollectionScan();

    /**
     * Implementation of FileMetadataWrite, see there. Calling these methods is equivalent.
     */
    void finishFileMetadataWrite(const ItemInfo& info, bool changed);

Q_SIGNALS:

    void collectionScanFinished();
    void newImages(const ItemInfoList&);
    void partialScanDone(const QString& path);
    void completeScanDone();
    void completeScanCanceled();
    void errorFromInitialization(const QString&);

private Q_SLOTS:

    void slotCancelPressed();
    void slotRelaxedScanning();

private:

    void finishedSchemaUpdate(UpdateResult result) override;

    //@}

private:

    // Disable
    ScanController(QObject*)                         = delete;
    ScanController(const ScanController&)            = delete;
    ScanController& operator=(const ScanController&) = delete;

private:

    friend class ScanControllerCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SCAN_CONTROLLER_H
