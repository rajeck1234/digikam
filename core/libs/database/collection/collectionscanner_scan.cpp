/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-21
 * Description : Collection scanning to database - scan operations.
 *
 * SPDX-FileCopyrightText: 2005-2006 by Tom Albers <tomalbers at kde dot nl>
 * SPDX-FileCopyrightText: 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "collectionscanner_p.h"

namespace Digikam
{

void CollectionScanner::completeScan()
{
    QElapsedTimer timer;
    timer.start();

    Q_EMIT startCompleteScan();

    {
        // lock database

        CoreDbTransaction transaction;

        mainEntryPoint(true);
        d->resetRemovedItemsTime();
    }

    // TODO: Implement a mechanism to watch for album root changes while we keep this list

    QList<CollectionLocation> allLocations = CollectionManager::instance()->allAvailableLocations();

    if (d->wantSignals && d->needTotalFiles)
    {
        // count for progress info

        int count = 0;

        Q_FOREACH (const CollectionLocation& location, allLocations)
        {
            // cppcheck-suppress useStlAlgorithm
            count += countItemsInFolder(location.albumRootPath());
        }

        Q_EMIT totalFilesToScan(count);
    }

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    // if we have no hints to follow, clean up all stale albums

    if (!d->hints || !d->hints->hasAlbumHints())
    {
        CoreDbAccess().db()->deleteStaleAlbums();
    }

    scanForStaleAlbums(allLocations);

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbumRoots();
    }

    Q_FOREACH (const CollectionLocation& location, allLocations)
    {
        scanAlbumRoot(location);
    }

    // do not continue to clean up without a complete scan!

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->deferredFileScanning)
    {
        qCDebug(DIGIKAM_DATABASE_LOG) << "Complete scan (file scanning deferred) took:" << timer.elapsed() << "msecs.";

        Q_EMIT finishedCompleteScan();

        return;
    }

    CoreDbTransaction transaction;
    completeScanCleanupPart();

    qCDebug(DIGIKAM_DATABASE_LOG) << "Complete scan took:" << timer.elapsed() << "msecs.";
}

void CollectionScanner::finishCompleteScan(const QStringList& albumPaths)
{
    Q_EMIT startCompleteScan();

    {
        // lock database

        CoreDbTransaction transaction;

        mainEntryPoint(true);
        d->resetRemovedItemsTime();
    }

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbumRoots();
    }

    // remove subalbums from list if parent album is already contained

    QStringList sortedPaths = albumPaths;
    std::sort(sortedPaths.begin(), sortedPaths.end());
    QStringList::iterator it, it2;

    for (it = sortedPaths.begin() ; it != sortedPaths.end() ; )
    {
        // remove all following entries as long as they have the same beginning (= are subalbums)

        for (it2 = it + 1 ; it2 != sortedPaths.end() && it2->startsWith(*it) ; )
        {
            it2 = sortedPaths.erase(it2);
        }

        it = it2;
    }

    if (d->wantSignals && d->needTotalFiles)
    {
        // count for progress info

        int count = 0;

        Q_FOREACH (const QString& path, sortedPaths)
        {
            // cppcheck-suppress useStlAlgorithm
            count += countItemsInFolder(path);
        }

        Q_EMIT totalFilesToScan(count);
    }

    Q_FOREACH (const QString& path, sortedPaths)
    {
        CollectionLocation location = CollectionManager::instance()->locationForPath(path);
        QString album               = CollectionManager::instance()->album(path);

        if (album == QLatin1String("/"))
        {
            scanAlbumRoot(location);
        }
        else
        {
            scanAlbum(location, album);
        }
    }

    // do not continue to clean up without a complete scan!

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    CoreDbTransaction transaction;
    completeScanCleanupPart();
}

void CollectionScanner::completeScanCleanupPart()
{
    completeHistoryScanning();

    updateRemovedItemsTime();

    // Items may be set to status removed, without being definitely deleted.
    // This deletion shall be done after a certain time, as checked by checkedDeleteRemoved

    if (checkDeleteRemoved())
    {
        // Mark items that are old enough and have the status trashed as obsolete
        // Only do this in a complete scan!

        CoreDbAccess access;
        QList<qlonglong> trashedItems = access.db()->getImageIds(DatabaseItem::Status::Trashed);

        Q_FOREACH (const qlonglong& item, trashedItems)
        {
            access.db()->setItemStatus(item, DatabaseItem::Status::Obsolete);
        }

        resetDeleteRemovedSettings();
    }
    else
    {
        // increment the count of complete scans during which removed items were not deleted

        incrementDeleteRemovedCompleteScanCount();
    }

    markDatabaseAsScanned();

    Q_EMIT finishedCompleteScan();
}

void CollectionScanner::partialScan(const QString& filePath)
{
    QString albumRoot = CollectionManager::instance()->albumRootPath(filePath);
    QString album     = CollectionManager::instance()->album(filePath);
    partialScan(albumRoot, album);
}

void CollectionScanner::partialScan(const QString& albumRoot, const QString& album)
{
    if (albumRoot.isNull() || album.isEmpty())
    {
        // If you want to scan the album root, pass "/"

        qCWarning(DIGIKAM_DATABASE_LOG) << "partialScan(QString, QString) called with invalid values";

        return;
    }

/*
    if (CoreDbAccess().backend()->isInTransaction())
    {
        // Install ScanController::instance()->suspendCollectionScan around your CoreDbTransaction

        qCDebug(DIGIKAM_DATABASE_LOG) << "Detected an active database transaction when starting a collection scan. "
                         "Please report this error.";

        return;
    }
*/

    mainEntryPoint(false);
    d->resetRemovedItemsTime();

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Did not find a CollectionLocation for album root path " << albumRoot;

        return;
    }

    // if we have no hints to follow, clean up all stale albums
    // Hint: Rethink with next major db update

    if (!d->hints || !d->hints->hasAlbumHints())
    {
        CoreDbAccess().db()->deleteStaleAlbums();
    }

    // Usually, we can restrict stale album scanning to our own location.
    // But when there are album hints from a second location to this location,
    // also scan the second location

    QSet<int> locationIdsToScan;
    locationIdsToScan << location.id();

    if (d->hints)
    {
        QReadLocker locker(&d->hints->lock);
        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>::const_iterator it;

        for (it = d->hints->albumHints.constBegin() ; it != d->hints->albumHints.constEnd() ; ++it)
        {
            if (it.key().albumRootId == location.id())
            {
                locationIdsToScan << it.key().albumRootId;
            }
        }
    }

    scanForStaleAlbums(locationIdsToScan.values());

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    if (album == QLatin1String("/"))
    {
        scanAlbumRoot(location);
    }
    else
    {
        scanAlbum(location, album);
    }

    finishHistoryScanning();

    if (!d->checkObserver())
    {
        Q_EMIT cancelled();

        return;
    }

    updateRemovedItemsTime();
}

qlonglong CollectionScanner::scanFile(const QString& filePath, FileScanMode mode)
{
    QFileInfo info(filePath);
    QString dirPath   = info.path(); // strip off filename
    QString albumRoot = CollectionManager::instance()->albumRootPath(dirPath);

    if (albumRoot.isNull())
    {
        return -1;
    }

    QString album = CollectionManager::instance()->album(dirPath);

    return scanFile(albumRoot, album, info.fileName(), mode);
}

qlonglong CollectionScanner::scanFile(const QString& albumRoot, const QString& album,
                                      const QString& fileName, FileScanMode mode)
{
    if (album.isEmpty() || fileName.isEmpty())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "scanFile(QString, QString, QString) called with empty album or empty filename";

        return -1;
    }

    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootPath(albumRoot);

    if (location.isNull())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Did not find a CollectionLocation for album root path " << albumRoot;

        return -1;
    }

    QDir dir(location.albumRootPath() + album);
    QFileInfo fi(dir, fileName);

    if (!fi.exists())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "File given to scan does not exist" << albumRoot << album << fileName;

        return -1;
    }

    int albumId       = checkAlbum(location, album);
    qlonglong imageId = CoreDbAccess().db()->getImageId(albumId, fileName);
    imageId           = scanFile(fi, albumId, imageId, mode);

    return imageId;
}

void CollectionScanner::scanFile(const ItemInfo& info, FileScanMode mode)
{
    if (info.isNull() || !info.isLocationAvailable())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    scanFile(fi, info.albumId(), info.id(), mode);
}

qlonglong CollectionScanner::scanFile(const QFileInfo& fi, int albumId, qlonglong imageId, FileScanMode mode)
{
    mainEntryPoint(false);

    if (!d->nameFilters.contains(fi.suffix().toLower()))
    {
        return -1;
    }

    if (imageId == -1)
    {
        switch (mode)
        {
            case NormalScan:
            case ModifiedScan:
                imageId = scanNewFile(fi, albumId);
                break;

            case Rescan:
            case CleanScan:
                imageId = scanNewFileFullScan(fi, albumId);
                break;
        }
    }
    else
    {
        ItemScanInfo scanInfo = CoreDbAccess().db()->getItemScanInfo(imageId);

        switch (mode)
        {
            case NormalScan:
                scanFileNormal(fi, scanInfo);
                break;

            case ModifiedScan:
                scanModifiedFile(fi, scanInfo);
                break;

            case Rescan:
                rescanFile(fi, scanInfo);
                break;

            case CleanScan:
                cleanScanFile(fi, scanInfo);
                break;
        }
    }

    finishHistoryScanning();

    return imageId;
}

void CollectionScanner::scanAlbumRoot(const CollectionLocation& location)
{
    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbumRoot(location.albumRootPath());
    }

    QMap<QString, QDateTime>::const_iterator it;
    const QMap<QString, QDateTime>& pathDateMap = CoreDbAccess().db()->
                                    getAlbumModificationMap(location.id());
    bool useFastScan = MetaEngineSettings::instance()->settings().useFastScan;

    if (!useFastScan || !d->performFastScan || pathDateMap.isEmpty())
    {
        scanAlbum(location, QLatin1String("/"));
    }
    else
    {
        for (it = pathDateMap.constBegin() ; it != pathDateMap.constEnd() ; ++it)
        {
            QDateTime modified;
            QString   folder(location.albumRootPath() + it.key());

            if (d->albumDateCache.contains(folder))
            {
                modified = d->albumDateCache.value(folder);
            }
            else
            {
                modified = QFileInfo(folder).lastModified();
            }

            if (s_modificationDateEquals(modified, it.value()))
            {
                int albumID = CoreDbAccess().db()->getAlbumForPath(location.id(), it.key(), false);
                int counter = CoreDbAccess().db()->getNumberOfItemsInAlbum(albumID);

                d->scannedAlbums << albumID;

                if (d->wantSignals)
                {
                    Q_EMIT scannedFiles(counter + 1);
                }
            }
            else
            {
                scanAlbum(location, it.key(), true);
            }
        }
    }

    if (d->wantSignals)
    {
        Q_EMIT finishedScanningAlbumRoot(location.albumRootPath());
    }
}

void CollectionScanner::scanForStaleAlbums(const QList<CollectionLocation>& locations)
{
    QList<int> locationIdsToScan;

    Q_FOREACH (const CollectionLocation& location, locations)
    {
        locationIdsToScan << location.id();
    }

    scanForStaleAlbums(locationIdsToScan);
}

void CollectionScanner::scanForStaleAlbums(const QList<int>& locationIdsToScan)
{
    if (d->wantSignals)
    {
        Q_EMIT startScanningForStaleAlbums();
    }

    QList<AlbumShortInfo> albumList = CoreDbAccess().db()->getAlbumShortInfos();
    QList<int> toBeDeleted;
    int counter = 0;

    if (d->wantSignals && d->needTotalFiles)
    {
        Q_EMIT totalFilesToScan(albumList.count());
    }

    QList<AlbumShortInfo>::const_iterator it3;

    for (it3 = albumList.constBegin() ; it3 != albumList.constEnd() ; ++it3)
    {
        ++counter;

        if (d->wantSignals && counter && (counter % 10 == 0))
        {
            Q_EMIT scannedFiles(counter);
            counter = 0;
        }

        if (!locationIdsToScan.contains((*it3).albumRootId) || toBeDeleted.contains((*it3).id))
        {
            continue;
        }

        CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId((*it3).albumRootId);

        // Only handle albums on available locations

        if (location.isAvailable())
        {
            QFileInfo fileInfo(location.albumRootPath() + (*it3).relativePath);
            bool dirExist = (fileInfo.exists() && fileInfo.isDir());

            if (location.asQtCaseSensitivity() == Qt::CaseInsensitive)
            {
                if (dirExist && !(*it3).relativePath.endsWith(QLatin1Char('/')))
                {
                    QDir dir(fileInfo.dir());
                    dirExist = dir.entryList(QDir::Dirs |
                                             QDir::NoDotAndDotDot)
                                             .contains(fileInfo.fileName());
                }
            }

            // let digikam think that ignored directories got deleted
            // (if they already exist in the database, this will delete them)

            if (!dirExist || d->ignoreDirectory.contains(fileInfo.fileName()))
            {
                // We have an ignored album, all sub-albums have to be ignored

                QList<int> subAlbums = CoreDbAccess().db()->getAlbumAndSubalbumsForPath((*it3).albumRootId,
                                                                                        (*it3).relativePath);
                toBeDeleted      << subAlbums;
                d->scannedAlbums << subAlbums;
            }
            else
            {
                d->albumDateCache.insert(fileInfo.filePath(), fileInfo.lastModified());
            }
        }
    }

    // At this point, it is important to handle album renames.
    // We can still copy over album attributes later, but we cannot identify
    // the former album of removed images.
    // Just renaming the album is also much cheaper than rescanning all files.

    if (!toBeDeleted.isEmpty() && d->hints)
    {
        // shallow copy for reading without caring for locks

        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album> albumHints;
        {
            QReadLocker locker(&d->hints->lock);
            albumHints = d->hints->albumHints;
        }

        // go through all album copy/move hints

        QHash<CollectionScannerHints::DstPath, CollectionScannerHints::Album>::const_iterator it;
        int toBeDeletedIndex;

        for (it = albumHints.constBegin() ; it != albumHints.constEnd() ; ++it)
        {
            // if the src entry of a hint is found in toBeDeleted, we have a move/rename, no copy. Handle these here.

            toBeDeletedIndex = toBeDeleted.indexOf(it.value().albumId);

            // We must double check that not, for some reason, the target album has already been scanned.

            QList<AlbumShortInfo>::const_iterator it2;

            for (it2 = albumList.constBegin() ; it2 != albumList.constEnd() ; ++it2)
            {
                if ((it2->albumRootId  == it.key().albumRootId) &&
                    (it2->relativePath == it.key().relativePath))
                {
                    toBeDeletedIndex = -1;
                    break;
                }
            }

            if (toBeDeletedIndex != -1)
            {
                // check for existence of target

                CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(it.key().albumRootId);

                if (location.isAvailable())
                {
                    QFileInfo fileInfo(location.albumRootPath() + it.key().relativePath);
                    bool dirExist = (fileInfo.exists() && fileInfo.isDir());

                    if (location.asQtCaseSensitivity() == Qt::CaseInsensitive)
                    {
                        if (dirExist && !it.key().relativePath.endsWith(QLatin1Char('/')))
                        {
                            QDir dir(fileInfo.dir());
                            dirExist = dir.entryList(QDir::Dirs |
                                                     QDir::NoDotAndDotDot)
                                                     .contains(fileInfo.fileName());
                        }
                    }

                    if (dirExist)
                    {
                        // Just set a new root/relativePath to the album. Further scanning will care for all cases or error.

                        CoreDbAccess().db()->renameAlbum(it.value().albumId, it.key().albumRootId, it.key().relativePath);

                        // No need any more to delete the album

                        toBeDeleted.removeAt(toBeDeletedIndex);
                    }
                }
            }
        }
    }

    safelyRemoveAlbums(toBeDeleted);

    if (d->wantSignals)
    {
        Q_EMIT finishedScanningForStaleAlbums();
    }
}

void CollectionScanner::scanAlbum(const CollectionLocation& location, const QString& album, bool checkDate)
{
    // + Adds album if it does not yet exist in the db.
    // + Recursively scans subalbums of album.
    // + Adds files if they do not yet exist in the db.
    // + Marks stale files as removed

    QDir dir(location.albumRootPath() + album);

    if (!dir.exists() || !dir.isReadable())
    {
        qCWarning(DIGIKAM_DATABASE_LOG) << "Folder does not exist or is not readable: "
                                        << dir.path();
        return;
    }

    if (d->wantSignals)
    {
        Q_EMIT startScanningAlbum(location.albumRootPath(), album);
    }

    int albumID                          = checkAlbum(location, album);
    QDateTime albumDateTime              = QFileInfo(dir.path()).lastModified();
    QDateTime albumModified              = CoreDbAccess().db()->getAlbumModificationDate(albumID);

    if (checkDate && s_modificationDateEquals(albumDateTime, albumModified))
    {
        // mark album as scanned

        d->scannedAlbums << albumID;

        if (d->wantSignals)
        {
            Q_EMIT finishedScanningAlbum(location.albumRootPath(), album, 1);
        }

        return;
    }

    const QList<ItemScanInfo>& scanInfos = CoreDbAccess().db()->getItemScanInfos(albumID);
    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();
    QHash<QString, int> fileNameIndexHash;
    QSet<qlonglong> itemIdSet;

    // create a QHash filename -> index in list

    for (int i = 0 ; i < scanInfos.size() ; ++i)
    {
        fileNameIndexHash[scanInfos.at(i).itemName] = i;
        itemIdSet << scanInfos.at(i).id;
    }

    const QStringList& list = dir.entryList(QDir::Dirs    |
                                            QDir::Files   |
                                            QDir::NoDotAndDotDot,
                                            QDir::Name | QDir::DirsLast);

    int counter          = 0;
    bool updateAlbumDate = false;
    QDate albumDateOld   = albumDateTime.date();
    QDate albumDateNew   = albumDateTime.date();
    const QString xmpExt(QLatin1String(".xmp"));

    Q_FOREACH (const QString& entry, list)
    {
        if (!d->checkObserver())
        {
            return; // return directly, do not go to cleanup code after loop!
        }

        QFileInfo info(dir, entry);

        if (info.isFile())
        {
            // filter with name filter

            if (!d->nameFilters.contains(info.suffix().toLower()))
            {
                continue;
            }

            ++counter;

            if (d->wantSignals && counter && (counter % 100 == 0))
            {
                Q_EMIT scannedFiles(counter);
                counter = 0;
            }

            int index = fileNameIndexHash.value(info.fileName(), -1);

            if      (index != -1)
            {
                // mark item as "seen"

                itemIdSet.remove(scanInfos.at(index).id);

                bool hasSidecar = (settings.useXMPSidecar4Reading                  &&
                                   (list.contains(info.fileName() + xmpExt)        ||
                                    list.contains(info.completeBaseName() + xmpExt)));

                scanFileNormal(info, scanInfos.at(index), hasSidecar);
            }
            else if (info.completeSuffix().contains(QLatin1String("digikamtempfile.")))
            {
                // ignore temp files we created ourselves

                continue;
            }
            else
            {
                // Read the creation date of each image to determine the oldest one

                qlonglong imageId = scanNewFile(info, albumID);

                ItemInfo itemInfo(imageId);
                QDate itemDate    = itemInfo.dateTime().date();

                if (itemDate.isValid())
                {
                    if ((settings.albumDateFrom == MetaEngineSettingsContainer::NewestItemDate) ||
                        (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate))
                    {
                        // Change album date only if the item date is newer.

                        if (itemDate > albumDateNew)
                        {
                            albumDateNew    = itemDate;
                            updateAlbumDate = true;
                        }
                    }

                    if ((settings.albumDateFrom == MetaEngineSettingsContainer::OldestItemDate) ||
                        (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate))
                    {
                        // Change album date only if the item date is older.

                        if (itemDate < albumDateOld)
                        {
                            albumDateOld    = itemDate;
                            updateAlbumDate = true;
                        }
                    }
                }

                // Q_EMIT signals for scanned files with much higher granularity

                if (d->wantSignals && counter && (counter % 2 == 0))
                {
                    Q_EMIT scannedFiles(counter);
                    counter = 0;
                }
            }
        }
        else if (info.isDir())
        {

#ifdef Q_OS_WIN

            // Hide album that starts with a dot, as under Linux.

            if (info.fileName().startsWith(QLatin1Char('.')))
            {
                continue;
            }

#endif

            if (d->ignoreDirectory.contains(info.fileName()))
            {
                continue;
            }

            ++counter;

            QString subAlbum = album;

            if (subAlbum != QLatin1String("/"))
            {
                subAlbum += QLatin1Char('/');
            }

            scanAlbum(location, subAlbum + info.fileName(), checkDate);
        }
    }

    if (!d->deferredFileScanning && !s_modificationDateEquals(albumDateTime, albumModified))
    {
        CoreDbAccess().db()->setAlbumModificationDate(albumID, albumDateTime);
    }

    if (updateAlbumDate)
    {
        // Write the new album date from the image information

        if      (settings.albumDateFrom == MetaEngineSettingsContainer::OldestItemDate)
        {
            CoreDbAccess().db()->setAlbumDate(albumID, albumDateOld);
        }
        else if (settings.albumDateFrom == MetaEngineSettingsContainer::NewestItemDate)
        {
            CoreDbAccess().db()->setAlbumDate(albumID, albumDateNew);
        }
        else if (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate)
        {
            qint64 julianDayCount = albumDateOld.toJulianDay();
            julianDayCount       += albumDateNew.toJulianDay();

            CoreDbAccess().db()->setAlbumDate(albumID, QDate::fromJulianDay(julianDayCount / 2));
        }
        else if (settings.albumDateFrom == MetaEngineSettingsContainer::FolderDate)
        {
            CoreDbAccess().db()->setAlbumDate(albumID, albumDateTime.date());
        }
    }

    if (d->wantSignals && counter)
    {
        Q_EMIT scannedFiles(counter);
    }

    // Mark items in the db which we did not see on disk.

    if (!itemIdSet.isEmpty())
    {
        QList<qlonglong> ids = itemIdSet.values();
        CoreDbOperationGroup group;
        CoreDbAccess().db()->removeItems(ids, QList<int>() << albumID);
        itemsWereRemoved(ids);
    }

    // mark album as scanned

    d->scannedAlbums << albumID;

    if (d->wantSignals)
    {
        Q_EMIT finishedScanningAlbum(location.albumRootPath(), album, list.count());
    }
}

void CollectionScanner::scanFileNormal(const QFileInfo& fi, const ItemScanInfo& scanInfo, bool checkSidecar)
{
    bool hasAnyHint = d->hints && d->hints->hasAnyNormalHint(scanInfo.id);

    // if the date is null, this signals a full rescan

    if (scanInfo.modificationDate.isNull() ||
        (hasAnyHint && d->hints->hasRescanHint(scanInfo.id)))
    {
        if (hasAnyHint)
        {
            QWriteLocker locker(&d->hints->lock);
            d->hints->rescanItemHints.remove(scanInfo.id);
        }

        rescanFile(fi, scanInfo);

        return;
    }
    else if (hasAnyHint && d->hints->hasModificationHint(scanInfo.id))
    {
        {
            QWriteLocker locker(&d->hints->lock);
            d->hints->modifiedItemHints.remove(scanInfo.id);
        }

        scanModifiedFile(fi, scanInfo);

        return;
    }
    else if (hasAnyHint) // metadata adjustment hints
    {
        if (d->hints->hasMetadataAboutToAdjustHint(scanInfo.id))
        {
            // postpone scan

            return;
        }
        else // hasMetadataAdjustedHint
        {
            {
                QWriteLocker locker(&d->hints->lock);
                d->hints->metadataAdjustedHints.remove(scanInfo.id);
            }

            scanFileUpdateHashReuseThumbnail(fi, scanInfo, true);

            return;
        }
    }
    else if (d->updatingHashHint)
    {
        // if the file need not be scanned because of modification, update the hash

        if (s_modificationDateEquals(fi.lastModified(), scanInfo.modificationDate) &&
            (fi.size() == scanInfo.fileSize))
        {
            scanFileUpdateHashReuseThumbnail(fi, scanInfo, false);

            return;
        }
    }

    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();
    QDateTime modificationDate           = fi.lastModified();

    if (checkSidecar                   &&
        settings.updateFileTimeStamp   &&
        settings.useXMPSidecar4Reading &&
        DMetadata::hasSidecar(fi.filePath()))
    {
        QString filePath      = DMetadata::sidecarPath(fi.filePath());
        QDateTime sidecarDate = QFileInfo(filePath).lastModified();

        if (sidecarDate > modificationDate)
        {
            modificationDate = sidecarDate;
        }
    }

    if (!s_modificationDateEquals(modificationDate, scanInfo.modificationDate) ||
        (fi.size() != scanInfo.fileSize))
    {
        if (settings.rescanImageIfModified)
        {
            cleanScanFile(fi, scanInfo);
        }
        else
        {
            scanModifiedFile(fi, scanInfo);
        }
    }
}

qlonglong CollectionScanner::scanNewFile(const QFileInfo& info, int albumId)
{
    if (d->checkDeferred(info))
    {
        return -1;
    }

    ItemScanner scanner(info);
    scanner.setCategory(category(info));

    // Check copy/move hints for single items

    qlonglong srcId = 0;

    if (d->hints)
    {
        QReadLocker locker(&d->hints->lock);
        srcId = d->hints->itemHints.value(NewlyAppearedFile(albumId, info.fileName()));
    }

    if (srcId != 0)
    {
        scanner.copiedFrom(albumId, srcId);
    }
    else
    {
        // Check copy/move hints for whole albums

        int srcAlbum = d->establishedSourceAlbums.value(albumId);

        if (srcAlbum)
        {
            // if we have one source album, find out if there is a file with the same name

            srcId = CoreDbAccess().db()->getImageId(srcAlbum, info.fileName());
        }

        if (srcId != 0)
        {
            scanner.copiedFrom(albumId, srcId);
        }
        else
        {
            // Establishing identity with the unique hash

            scanner.newFile(albumId);
        }
    }

    d->finishScanner(scanner);
    d->newIdsList << scanner.id();

    return scanner.id();
}

qlonglong CollectionScanner::scanNewFileFullScan(const QFileInfo& info, int albumId)
{
    if (d->checkDeferred(info))
    {
        return -1;
    }

    ItemScanner scanner(info);
    scanner.setCategory(category(info));
    scanner.newFileFullScan(albumId);
    d->finishScanner(scanner);

    return scanner.id();
}

void CollectionScanner::scanModifiedFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.fileModified();
    d->finishScanner(scanner);
}

void CollectionScanner::scanFileUpdateHashReuseThumbnail(const QFileInfo& info, const ItemScanInfo& scanInfo,
                                                         bool fileWasEdited)
{
    QString oldHash   = scanInfo.uniqueHash;
    qlonglong oldSize = scanInfo.fileSize;

    // same code as scanModifiedFile

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.fileModified();

    QString newHash   = scanner.itemScanInfo().uniqueHash;
    qlonglong newSize = scanner.itemScanInfo().fileSize;

    if (ThumbsDbAccess::isInitialized())
    {
        if (fileWasEdited)
        {
            // The file was edited in such a way that we know that the pixel content did not change, so we can reuse the thumbnail.
            // We need to add a link to the thumbnail data with the new hash/file size _and_ adjust
            // the file modification date in the data table.

            ThumbsDbInfo thumbDbInfo = ThumbsDbAccess().db()->findByHash(oldHash, oldSize);

            if (thumbDbInfo.id != -1)
            {
                ThumbsDbAccess().db()->insertUniqueHash(newHash, newSize, thumbDbInfo.id);
                ThumbsDbAccess().db()->updateModificationDate(thumbDbInfo.id, scanner.itemScanInfo().modificationDate);

                // TODO: also update details thumbnails (by file path and URL scheme)
            }
        }
        else
        {
            ThumbsDbAccess().db()->replaceUniqueHash(oldHash, oldSize, newHash, newSize);
        }
    }

    d->finishScanner(scanner);
}

void CollectionScanner::cleanScanFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.cleanScan();
    d->finishScanner(scanner);
}

void CollectionScanner::rescanFile(const QFileInfo& info, const ItemScanInfo& scanInfo)
{
    if (d->checkDeferred(info))
    {
        return;
    }

    ItemScanner scanner(info, scanInfo);
    scanner.setCategory(category(info));
    scanner.rescan();
    d->finishScanner(scanner);
}

void CollectionScanner::completeHistoryScanning()
{
    // scan tagged images

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    QList<qlonglong> ids = CoreDbAccess().db()->getItemIDsInTag(needResolvingTag);
    historyScanningStage2(ids);

    ids                  = CoreDbAccess().db()->getItemIDsInTag(needTaggingTag);
    qCDebug(DIGIKAM_DATABASE_LOG) << "items to tag" << ids;
    historyScanningStage3(ids);
}

void CollectionScanner::finishHistoryScanning()
{
    // scan recorded ids

    QList<qlonglong> ids;

    // stage 2

    ids = d->needResolveHistorySet.values();
    d->needResolveHistorySet.clear();
    historyScanningStage2(ids);

    if (!d->checkObserver())
    {
        return;
    }

    // stage 3

    ids = d->needTaggingHistorySet.values();
    d->needTaggingHistorySet.clear();
    historyScanningStage3(ids);
}

void CollectionScanner::historyScanningStage2(const QList<qlonglong>& ids)
{
    Q_FOREACH (const qlonglong& id, ids)
    {
        if (!d->checkObserver())
        {
            return;
        }

        CoreDbOperationGroup group;

        if (d->recordHistoryIds)
        {
            QList<qlonglong> needTaggingIds;
            ItemScanner::resolveImageHistory(id, &needTaggingIds);

            Q_FOREACH (const qlonglong& needTag, needTaggingIds)
            {
                d->needTaggingHistorySet << needTag;
            }
        }
        else
        {
            ItemScanner::resolveImageHistory(id);
        }
    }
}

void CollectionScanner::historyScanningStage3(const QList<qlonglong>& ids)
{
    Q_FOREACH (const qlonglong& id, ids)
    {
        if (!d->checkObserver())
        {
            return;
        }

        CoreDbOperationGroup group;
        ItemScanner::tagItemHistoryGraph(id);
    }
}

bool CollectionScanner::databaseInitialScanDone()
{
    CoreDbAccess access;

    return !access.db()->getSetting(QLatin1String("Scanned")).isEmpty();
}

} // namespace Digikam
