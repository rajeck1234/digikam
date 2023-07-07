/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-05-17
 * Description : low level files management interface.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018      by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dio.h"

// Qt includes

#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <QAbstractButton>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "iteminfo.h"
#include "diofinders.h"
#include "albummanager.h"
#include "tagscache.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "coredbtransaction.h"
#include "album.h"
#include "dmetadata.h"
#include "metaenginesettings.h"
#include "scancontroller.h"
#include "thumbsdb.h"
#include "thumbsdbaccess.h"
#include "iojobsmanager.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "dnotificationwrapper.h"
#include "loadingcacheinterface.h"
#include "progressmanager.h"
#include "digikamapp.h"
#include "iojobdata.h"

namespace Digikam
{

class Q_DECL_HIDDEN DIOCreator
{
public:

    DIO object;
};

Q_GLOBAL_STATIC(DIOCreator, creator)

// ------------------------------------------------------------------------------------------------

DIO* DIO::instance()
{
    return &creator->object;
}

DIO::DIO()
{
    m_processingCount = 0;
}

DIO::~DIO()
{
}

void DIO::cleanUp()
{
}

bool DIO::itemsUnderProcessing()
{
    return instance()->m_processingCount;
}

// Album -> Album -----------------------------------------------------

void DIO::copy(PAlbum* const src, PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyAlbum, src, dest));
}

void DIO::move(PAlbum* const src, PAlbum* const dest)
{
    if (!src || !dest)
    {
        return;
    }

#ifdef Q_OS_WIN

    AlbumManager::instance()->removeWatchedPAlbums(src);

#endif

    instance()->processJob(new IOJobData(IOJobData::MoveAlbum, src, dest));
}

// Images -> Album ----------------------------------------------------

void DIO::copy(const QList<ItemInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyImage, infos, dest));
}

void DIO::move(const QList<ItemInfo>& infos, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::MoveImage, infos, dest));
}

// External files -> album --------------------------------------------

void DIO::copy(const QUrl& src, PAlbum* const dest)
{
    copy(QList<QUrl>() << src, dest);
}

void DIO::copy(const QList<QUrl>& srcList, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::CopyFiles, srcList, dest));
}

void DIO::move(const QUrl& src, PAlbum* const dest)
{
    move(QList<QUrl>() << src, dest);
}

void DIO::move(const QList<QUrl>& srcList, PAlbum* const dest)
{
    if (!dest)
    {
        return;
    }

    instance()->processJob(new IOJobData(IOJobData::MoveFiles, srcList, dest));
}

// Images -> External -------------------------------------------------

void DIO::copy(const QList<ItemInfo>& infos, const QUrl& dest)
{
    instance()->processJob(new IOJobData(IOJobData::CopyToExt, infos, dest));
}

// Rename -------------------------------------------------------------

void DIO::rename(const QUrl& src, const QString& newName, bool overwrite)
{
    if (src.isEmpty() || newName.isEmpty())
    {
        return;
    }

    ItemInfo info = ItemInfo::fromUrl(src);

    instance()->processJob(new IOJobData(IOJobData::Rename, info, newName, overwrite));
}

// Delete -------------------------------------------------------------

void DIO::del(const QList<ItemInfo>& infos, bool useTrash)
{
    instance()->processJob(new IOJobData(useTrash ? IOJobData::Trash
                                                  : IOJobData::Delete, infos));
}

void DIO::del(const ItemInfo& info, bool useTrash)
{
    del(QList<ItemInfo>() << info, useTrash);
}

void DIO::del(PAlbum* const album, bool useTrash)
{
    if (!album)
    {
        return;
    }

#ifdef Q_OS_WIN

    AlbumManager::instance()->removeWatchedPAlbums(album);

#endif

    instance()->createJob(new IOJobData(useTrash ? IOJobData::Trash
                                                 : IOJobData::Delete, album));
}

// Restore Trash ------------------------------------------------------

void DIO::restoreTrash(const DTrashItemInfoList& infos)
{
    instance()->createJob(new IOJobData(IOJobData::Restore, infos));
}

// Empty Trash --------------------------------------------------------

void DIO::emptyTrash(const DTrashItemInfoList& infos)
{
    instance()->createJob(new IOJobData(IOJobData::Empty, infos));
}

// ------------------------------------------------------------------------------------------------

void DIO::processJob(IOJobData* const data)
{
    const int operation = data->operation();

    if      ((operation == IOJobData::CopyImage) ||
             (operation == IOJobData::MoveImage))
    {
        // this is a fast db operation, do here

        GroupedImagesFinder finder(data->itemInfos());
        data->setItemInfos(finder.infos);
    }
    else if (operation == IOJobData::CopyAlbum)
    {
        ScanController::instance()->hintAtMoveOrCopyOfAlbum(data->srcAlbum(), data->destAlbum());
        createJob(data);

        return;
    }
    else if ((operation == IOJobData::Trash) ||
             (operation == IOJobData::Delete))
    {
        SidecarFinder finder(data->sourceUrls());
        data->setSourceUrls(finder.localFiles);

        qCDebug(DIGIKAM_DATABASE_LOG) << "Number of files to be deleted:" << data->sourceUrls().count();
    }

    if (operation == IOJobData::Rename)
    {
        SidecarFinder finder(data->sourceUrls());
        data->setSourceUrls(finder.localFiles);

        for (int i = 0 ; i < finder.localFiles.size() ; ++i)
        {
            if (finder.localFileModes.at(i))
            {
                data->setDestUrl(finder.localFiles.at(i),
                                 QUrl::fromLocalFile(data->destUrl().toLocalFile() +
                                                     finder.localFileSuffixes.at(i)));
            }
            else
            {
                QFileInfo basInfo(data->destUrl().toLocalFile());

                data->setDestUrl(finder.localFiles.at(i),
                                 QUrl::fromLocalFile(basInfo.path()             +
                                                     QLatin1Char('/')           +
                                                     basInfo.completeBaseName() +
                                                     finder.localFileSuffixes.at(i)));
            }
        }
    }

    createJob(data);
}

void DIO::createJob(IOJobData* const data)
{
    if (data->sourceUrls().isEmpty())
    {
        delete data;

        return;
    }

    const int operation = data->operation();

    if ((operation == IOJobData::CopyImage) || (operation == IOJobData::CopyAlbum) ||
        (operation == IOJobData::CopyFiles) || (operation == IOJobData::CopyToExt) ||
        (operation == IOJobData::MoveImage) || (operation == IOJobData::MoveAlbum) ||
        (operation == IOJobData::MoveFiles))
    {
        CollectionLocation location         = CollectionManager::instance()->locationForUrl(data->destUrl());
        Qt::CaseSensitivity caseSensitivity = location.asQtCaseSensitivity();
        QDir dir(data->destUrl().toLocalFile());

        const QStringList& dirList          = dir.entryList(QDir::Dirs    |
                                                            QDir::Files   |
                                                            QDir::NoDotAndDotDot);

        Q_FOREACH (const QUrl& url, data->sourceUrls())
        {
            if (dirList.contains(url.adjusted(QUrl::StripTrailingSlash).fileName(), caseSensitivity))
            {
                QPointer<QMessageBox> msgBox = new QMessageBox(QMessageBox::Warning,
                        i18nc("@title:window", "File Conflict"),
                        i18n("Files or folders with the same name already exist in the target folder.\n\n"
                             "What action is applied in the event of a file conflict?\n\n"
                             "Available options are:\n"
                             "Rename automatically: conflicting files will be renamed.\n"
                             "Overwrite automatically: conflicting files will be overwritten.\n"
                             "Skip automatically: conflicting files will be skipped."),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Ok | QMessageBox::Cancel,
                        qApp->activeWindow());

                msgBox->button(QMessageBox::Yes)->setText(i18nc("@action:button", "Rename Automatically"));
                msgBox->button(QMessageBox::Yes)->setIcon(QIcon::fromTheme(QLatin1String("document-edit")));
                msgBox->button(QMessageBox::No)->setText(i18nc("@action:button", "Overwrite Automatically"));
                msgBox->button(QMessageBox::No)->setIcon(QIcon::fromTheme(QLatin1String("edit-copy")));
                msgBox->button(QMessageBox::Ok)->setText(i18nc("@action:button", "Skip Automatically"));
                msgBox->button(QMessageBox::Ok)->setIcon(QIcon::fromTheme(QLatin1String("go-next")));

                if ((operation == IOJobData::CopyAlbum) || (operation == IOJobData::MoveAlbum))
                {
                    msgBox->button(QMessageBox::No)->hide();
                }

                int result = msgBox->exec();
                delete msgBox;

                if      (result == QMessageBox::Cancel)
                {
                    delete data;

                    return;
                }
                else if (result == QMessageBox::Yes)
                {
                    data->setFileConflict(IOJobData::AutoRename);
                }
                else if (result == QMessageBox::No)
                {
                    data->setFileConflict(IOJobData::Overwrite);
                }

                break;
            }
        }
    }

    ProgressItem* item = nullptr;
    QString itemString = getItemString(data);

    if (!itemString.isEmpty())
    {
        item = ProgressManager::instance()->createProgressItem(itemString,
                                                               QString(), true, false);
        item->setTotalItems(data->sourceUrls().count());
        data->setProgressId(item->id());
    }

    IOJobsThread* const jobThread = IOJobsManager::instance()->startIOJobs(data);

    connect(jobThread, SIGNAL(signalOneProccessed(QUrl)),
            this, SLOT(slotOneProccessed(QUrl)));

    connect(jobThread, SIGNAL(signalFinished()),
            this, SLOT(slotResult()));

    if (operation == IOJobData::Rename)
    {
        connect(jobThread, SIGNAL(signalRenameFailed(QUrl)),
                this, SIGNAL(signalRenameFailed(QUrl)));

        connect(jobThread, SIGNAL(signalFinished()),
                this, SIGNAL(signalRenameFinished()));
    }

    if ((operation == IOJobData::Empty) ||
        (operation == IOJobData::Restore))
    {
        connect(jobThread, SIGNAL(signalFinished()),
                this, SIGNAL(signalTrashFinished()));
    }

    if (item)
    {
        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                jobThread, SLOT(slotCancel()));

        connect(item, SIGNAL(progressItemCanceled(ProgressItem*)),
                this, SLOT(slotCancel(ProgressItem*)));
    }

    ++m_processingCount;
}

void DIO::slotResult()
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread || !jobThread->jobData())
    {
        return;
    }

    IOJobData* const data = jobThread->jobData();
    const int operation   = data->operation();

    if      (jobThread->hasErrors() && operation != IOJobData::Rename)
    {
        // Pop-up a message about the error.

        QString errors = jobThread->errorsList().join(QLatin1Char('\n'));
        DNotificationWrapper(QString(), errors, DigikamApp::instance(),
                             DigikamApp::instance()->windowTitle());
    }
    else
    {
        if ((operation == IOJobData::CopyImage) ||
            (operation == IOJobData::MoveImage))
        {
            if (data->destAlbum())
            {
                updateAlbumDate(data->destAlbum()->id());
            }
        }

        if ((operation == IOJobData::Trash)  ||
            (operation == IOJobData::Delete) ||
            (operation == IOJobData::MoveImage))
        {
            Q_FOREACH (int albumID, data->srcAlbumIds())
            {
                updateAlbumDate(albumID);
            }
        }
    }

    if (m_processingCount)
    {
        --m_processingCount;
    }

    slotCancel(getProgressItem(data));
}

void DIO::slotOneProccessed(const QUrl& url)
{
    IOJobsThread* const jobThread = dynamic_cast<IOJobsThread*>(sender());

    if (!jobThread || !jobThread->jobData())
    {
        return;
    }

    IOJobData* const data = jobThread->jobData();
    const int operation   = data->operation();

    switch (operation)
    {
        case IOJobData::CopyImage:
        {
            ItemInfo info = data->findItemInfo(url);

            if (!info.isNull() && data->destAlbum())
            {
                CoreDbAccess access;
                CoreDbTransaction transaction(&access);

                qlonglong id = access.db()->copyItem(info.albumId(), info.name(),
                                                     data->destAlbum()->id(), data->destName(url));

                // Remove grouping for copied items.

                access.db()->removeAllImageRelationsFrom(id, DatabaseRelation::Grouped);
                access.db()->removeAllImageRelationsTo(id, DatabaseRelation::Grouped);
            }

            break;
        }

        case IOJobData::MoveImage:
        {
            ItemInfo info = data->findItemInfo(url);

            if (!info.isNull() && data->destAlbum())
            {
                CoreDbAccess().db()->moveItem(info.albumId(), info.name(),
                                              data->destAlbum()->id(), data->destName(url));
            }

            break;
        }

        case IOJobData::CopyFiles:
        case IOJobData::MoveFiles:
        {
            QString scanPath = data->destUrl().adjusted(QUrl::StripTrailingSlash).toLocalFile();
            scanPath        += QLatin1Char('/') + data->destName(url);

            QFileInfo scanInfo(scanPath);

            if (scanInfo.isDir())
            {
                ScanController::instance()->scheduleCollectionScanRelaxed(scanPath);
            }
            else
            {
                ScanController::instance()->scannedInfo(scanPath);
            }

            break;

        }

        case IOJobData::CopyToExt:
        {
            CollectionLocation location = CollectionManager::instance()->locationForUrl(data->destUrl());

            // The target of the copy is within the digiKam collections?

            if (!location.isNull())
            {
                QString filePath = data->destUrl().adjusted(QUrl::StripTrailingSlash).toLocalFile();
                filePath        += QLatin1Char('/') + data->destName(url);
                ScanController::instance()->scannedInfo(filePath);
            }

            break;
        }

        case IOJobData::CopyAlbum:
        {
            QString scanPath = data->destUrl().adjusted(QUrl::StripTrailingSlash).toLocalFile();
            ScanController::instance()->scheduleCollectionScanRelaxed(scanPath);

            break;
        }

        case IOJobData::MoveAlbum:
        {
            if (data->srcAlbum() && data->destAlbum())
            {
                CoreDbAccess access;
                QList<int> albumsToMove;
                QString newName  = data->destName(url);
                QString basePath = data->srcAlbum()->albumPath();
                QString destPath = data->destAlbum()->albumPath();

                if (!destPath.endsWith(QLatin1Char('/')))
                {
                    destPath.append(QLatin1Char('/'));
                }

                addAlbumChildrenToList(albumsToMove, data->srcAlbum());

                Q_FOREACH (int albumId, albumsToMove)
                {
                    QString relativePath = access.db()->getAlbumRelativePath(albumId);
                    relativePath         = relativePath.section(basePath, 1, -1);
                    relativePath         = destPath + newName + relativePath;
                    access.db()->renameAlbum(albumId, data->destAlbum()->albumRootId(), relativePath);
                }
            }

            break;
        }

        case IOJobData::Trash:
        case IOJobData::Delete:
        {
            // Mark the images as obsolete and remove them
            // from their album and from the grouped

            int originalVersionTag = TagsCache::instance()->
                getOrCreateInternalTag(InternalTagName::originalVersion());
            int needTaggingTag     = TagsCache::instance()->
                getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

            PAlbum* const album    = data->srcAlbum();

            if (album)
            {
                // Get all deleted albums

                CoreDbAccess access;
                CoreDbTransaction transaction(&access);

                QList<int> albumsToDelete;
                QList<qlonglong> imagesToRemove;

                addAlbumChildrenToList(albumsToDelete, album);

                Q_FOREACH (int albumId, albumsToDelete)
                {
                    imagesToRemove << access.db()->getItemIDsInAlbum(albumId);
                }

                Q_FOREACH (const qlonglong& removeId, imagesToRemove)
                {
                    const QList<qlonglong>& imageIds = access.db()->
                          getImagesRelatedFrom(removeId, DatabaseRelation::DerivedFrom);

                    Q_FOREACH (const qlonglong& id, imageIds)
                    {
                        access.db()->removeItemTag(id, originalVersionTag);
                        access.db()->addItemTag(id, needTaggingTag);
                    }

                    access.db()->removeAllImageRelationsFrom(removeId,
                                                             DatabaseRelation::Grouped);
                }

                if (operation == IOJobData::Trash)
                {
                    access.db()->removeItems(imagesToRemove, albumsToDelete);
                }
                else
                {
                    access.db()->removeItemsPermanently(imagesToRemove, albumsToDelete);
                }

                Q_FOREACH (int albumId, albumsToDelete)
                {
                    if (operation == IOJobData::Trash)
                    {
                        access.db()->makeStaleAlbum(albumId);
                    }
                    else
                    {
                        access.db()->deleteAlbum(albumId);
                    }
                }
            }
            else
            {
                ItemInfo info = data->findItemInfo(url);

                if (!info.isNull())
                {
                    CoreDbAccess access;
                    const QList<qlonglong>& imageIds = access.db()->
                          getImagesRelatedFrom(info.id(), DatabaseRelation::DerivedFrom);

                    Q_FOREACH (const qlonglong& id, imageIds)
                    {
                        access.db()->removeItemTag(id, originalVersionTag);
                        access.db()->addItemTag(id, needTaggingTag);
                    }

                    access.db()->removeAllImageRelationsFrom(info.id(),
                                                             DatabaseRelation::Grouped);

                    if (operation == IOJobData::Trash)
                    {
                        access.db()->removeItems(QList<qlonglong>() << info.id(),
                                                 QList<int>() << info.albumId());
                    }
                    else
                    {
                        access.db()->removeItemsPermanently(QList<qlonglong>() << info.id(),
                                                            QList<int>() << info.albumId());
                    }
                }
            }

            break;
        }

        case IOJobData::Rename:
        {
            ItemInfo info = data->findItemInfo(url);

            if (!info.isNull())
            {
                QString oldPath = url.toLocalFile();
                QString newName = data->destUrl(url).fileName();
                QString newPath = data->destUrl(url).toLocalFile();

                if (data->fileConflict() == IOJobData::Overwrite)
                {
                    ThumbsDbAccess().db()->removeByFilePath(newPath);
                    LoadingCacheInterface::fileChanged(newPath, false);
                    CoreDbAccess().db()->deleteItem(info.albumId(), newName);
                }

                ThumbsDbAccess().db()->renameByFilePath(oldPath, newPath);

                // Remove old thumbnails and images from the cache

                LoadingCacheInterface::fileChanged(oldPath, false);

                // Rename in ItemInfo and database

                info.setName(newName);
            }

            break;
        }

        case IOJobData::Restore:
        {
            ScanController::instance()->scannedInfo(url.toLocalFile());

            break;
        }

        default:
        {
            break;
        }
    }

    ProgressItem* const item = getProgressItem(data);

    if (item)
    {
        item->advance(1);
    }
}

void DIO::updateAlbumDate(int albumID)
{
    QDate newAlbumDate;
    MetaEngineSettingsContainer settings = MetaEngineSettings::instance()->settings();

    if      (settings.albumDateFrom == MetaEngineSettingsContainer::OldestItemDate)
    {
        newAlbumDate = CoreDbAccess().db()->getAlbumLowestDate(albumID);
    }
    else if (settings.albumDateFrom == MetaEngineSettingsContainer::NewestItemDate)
    {
        newAlbumDate = CoreDbAccess().db()->getAlbumHighestDate(albumID);
    }
    else if (settings.albumDateFrom == MetaEngineSettingsContainer::AverageDate)
    {
        newAlbumDate = CoreDbAccess().db()->getAlbumAverageDate(albumID);
    }

    if (newAlbumDate.isValid())
    {
        CoreDbAccess().db()->setAlbumDate(albumID, newAlbumDate);
    }
}

QString DIO::getItemString(IOJobData* const data) const
{
    switch (data->operation())
    {
        case IOJobData::CopyAlbum:
        {
            return i18n("Copy Album");
        }

        case IOJobData::CopyImage:
        {
            return i18n("Copy Images");
        }

        case IOJobData::CopyFiles:
        case IOJobData::CopyToExt:
        {
            return i18n("Copy Files");
        }

        case IOJobData::MoveAlbum:
        {
            return i18n("Move Album");
        }

        case IOJobData::MoveImage:
        {
            return i18n("Move Images");
        }

        case IOJobData::MoveFiles:
        {
            return i18n("Move Files");
        }

        case IOJobData::Delete:
        {
            return i18n("Delete");
        }

        case IOJobData::Trash:
        {
            return i18n("Trash");
        }

        case IOJobData::Restore:
        {
            return i18n("Restore Trash");
        }

        case IOJobData::Empty:
        {
            return i18n("Empty Trash");
        }

        default:
        {
            break;
        }
    }

    return QString();
}

ProgressItem* DIO::getProgressItem(IOJobData* const data) const
{
    QString itemId = data->getProgressId();

    if (itemId.isEmpty())
    {
        return nullptr;
    }

    return ProgressManager::instance()->findItembyId(itemId);
}

void DIO::slotCancel(ProgressItem* item)
{
    if (item)
    {
        item->setComplete();
    }
}

void DIO::addAlbumChildrenToList(QList<int>& list, Album* const album)
{
    // simple recursive helper function

    if (album)
    {
        if (!list.contains(album->id()))
        {
            list.append(album->id());
        }

        AlbumIterator it(album);

        while (it.current())
        {
            addAlbumChildrenToList(list, *it);
            ++it;
        }
    }
}

} // namespace Digikam
