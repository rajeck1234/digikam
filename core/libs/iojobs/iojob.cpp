/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-06-15
 * Description : IO Jobs for file systems jobs
 *
 * SPDX-FileCopyrightText: 2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2018 by Maik Qualmann <metzpinguin at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "iojob.h"

// Qt includes

#include <QDir>
#include <QFile>
#include <QDirIterator>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "iteminfo.h"
#include "dtrash.h"
#include "coredb.h"
#include "coredbaccess.h"
#include "albummanager.h"
#include "dfileoperations.h"
#include "coredboperationgroup.h"

namespace Digikam
{

IOJob::IOJob()
{
}

// --------------------------------------------

CopyOrMoveJob::CopyOrMoveJob(IOJobData* const data)
    : m_data(data)
{
}

void CopyOrMoveJob::run()
{
    while (m_data && !m_cancel)
    {
        QUrl srcUrl = m_data->getNextUrl();

        if (srcUrl.isEmpty())
        {
            break;
        }

        QFileInfo srcInfo(srcUrl.adjusted(QUrl::StripTrailingSlash).toLocalFile());
        QDir dstDir(m_data->destUrl().toLocalFile());
        QString srcName = srcInfo.fileName();

        if (!srcInfo.exists())
        {
            Q_EMIT signalError(i18n("File/Folder %1 does not exist anymore", srcName));
            continue;
        }

        if (!dstDir.exists())
        {
            Q_EMIT signalError(i18n("Album %1 does not exist anymore", dstDir.dirName()));
            continue;
        }

        // Checking if there is a file with the same name in destination folder

        QString destenation = dstDir.path() + QLatin1Char('/') + srcName;

        if (QFileInfo::exists(destenation))
        {
            if      (m_data->fileConflict() == IOJobData::Overwrite)
            {
                if (srcInfo.isDir())
                {
                    continue;
                }
                else
                {
                    if (srcInfo.filePath() == destenation)
                    {
                        continue;
                    }

                    if      (m_data->operation() == IOJobData::CopyToExt)
                    {
                        if (!QFile::remove(destenation))
                        {
                            Q_EMIT signalError(i18n("Could not overwrite image %1",
                                                    srcName));

                            continue;
                        }
                    }
                    else if (!DTrash::deleteImage(destenation, m_data->jobTime()))
                    {
                        Q_EMIT signalError(i18n("Could not move image %1 to collection trash.\n"
                                                "Check the file permission on the trash folder "
                                                "\"%2\" in the image collection.",
                                                srcName, DTrash::TRASH_FOLDER));

                        continue;
                    }
                }
            }
            else if (m_data->fileConflict() == IOJobData::AutoRename)
            {
                QUrl destUrl = QUrl::fromLocalFile(destenation);

                if (srcInfo.isDir())
                {
                    QUrl renamed = DFileOperations::getUniqueFolderUrl(destUrl);
                    destenation  = renamed.toLocalFile();
                    m_data->setDestUrl(srcUrl, renamed);
                }
                else
                {
                    QUrl renamed = DFileOperations::getUniqueFileUrl(destUrl);
                    destenation  = renamed.toLocalFile();
                    m_data->setDestUrl(srcUrl, renamed);
                }
            }
            else
            {
                Q_EMIT signalError(i18n("A file or folder named %1 already exists in %2",
                                        srcName, QDir::toNativeSeparators(dstDir.path())));

                continue;
            }
        }

        if ((m_data->operation() == IOJobData::MoveAlbum) ||
            (m_data->operation() == IOJobData::MoveImage) ||
            (m_data->operation() == IOJobData::MoveFiles))
        {
            if (srcInfo.isDir())
            {
                QDir srcDir(srcInfo.filePath());

                if (!srcDir.rename(srcDir.path(), destenation))
                {
                    // If QDir::rename fails, try copy and remove.

                    if      (!DFileOperations::copyFolderRecursively(srcDir.path(), dstDir.path(),
                                                                     m_data->getProgressId(), &m_cancel))
                    {
                        Q_EMIT signalOneProccessed(srcUrl);

                        if (m_cancel)
                        {
                            break;
                        }

                        Q_EMIT signalError(i18n("Could not move folder %1 to album %2",
                                                srcName, QDir::toNativeSeparators(dstDir.path())));

                        continue;
                    }
                    else if (!srcDir.removeRecursively())
                    {
                        Q_EMIT signalError(i18n("Could not move folder %1 to album %2. "
                                                "The folder %1 was copied as well to album %2",
                                                srcName, QDir::toNativeSeparators(dstDir.path())));
                    }
                }
            }
            else
            {
                // move the possible sidecar files first

                if (!DFileOperations::sidecarFiles(srcInfo.filePath(), destenation, DFileOperations::Rename))
                {
                    Q_EMIT signalError(i18n("Could not move sidecar from file %1 to album %2",
                                            srcName, QDir::toNativeSeparators(dstDir.path())));
                }

                if (!DFileOperations::renameFile(srcInfo.filePath(), destenation))
                {
                    Q_EMIT signalError(i18n("Could not move file %1 to album %2",
                                            srcName, QDir::toNativeSeparators(dstDir.path())));

                    continue;
                }
           }
        }
        else
        {
            if (srcInfo.isDir())
            {
                QDir srcDir(srcInfo.filePath());

                if (!DFileOperations::copyFolderRecursively(srcDir.path(), dstDir.path(),
                                                            m_data->getProgressId(), &m_cancel))
                {
                    Q_EMIT signalOneProccessed(srcUrl);

                    if (m_cancel)
                    {
                        break;
                    }

                    Q_EMIT signalError(i18n("Could not copy folder %1 to album %2",
                                            srcName, QDir::toNativeSeparators(dstDir.path())));

                    continue;
                }
            }
            else
            {
                // copy the possible sidecar files first

                if (!DFileOperations::sidecarFiles(srcInfo.filePath(), destenation, DFileOperations::Copy))
                {
                    Q_EMIT signalError(i18n("Could not copy sidecar from file %1 to folder %2",
                                            srcName, QDir::toNativeSeparators(dstDir.path())));
                }

                if (!DFileOperations::copyFile(srcInfo.filePath(), destenation))
                {
                    if (m_data->operation() == IOJobData::CopyToExt)
                    {
                        Q_EMIT signalError(i18n("Could not copy file %1 to folder %2",
                                                srcName, QDir::toNativeSeparators(dstDir.path())));
                    }
                    else
                    {
                        Q_EMIT signalError(i18n("Could not copy file %1 to album %2",
                                                srcName, QDir::toNativeSeparators(dstDir.path())));
                    }

                    continue;
                }
            }
        }

        Q_EMIT signalOneProccessed(srcUrl);
    }

    Q_EMIT signalDone();
}

// --------------------------------------------

DeleteJob::DeleteJob(IOJobData* const data)
    : m_data(data)
{
}

void DeleteJob::run()
{
    while (m_data && !m_cancel)
    {
        QUrl deleteUrl = m_data->getNextUrl();

        if (deleteUrl.isEmpty())
        {
            break;
        }

        bool useTrash = (m_data->operation() == IOJobData::Trash);

        QFileInfo fileInfo(deleteUrl.toLocalFile());
        qCDebug(DIGIKAM_IOJOB_LOG) << "Deleting:   " << fileInfo.filePath();
        qCDebug(DIGIKAM_IOJOB_LOG) << "File exists?" << fileInfo.exists();
        qCDebug(DIGIKAM_IOJOB_LOG) << "Is to trash?" << useTrash;

        if (!fileInfo.exists())
        {
            Q_EMIT signalError(i18n("File/Folder %1 does not exist",
                                    QDir::toNativeSeparators(fileInfo.filePath())));

            continue;
        }

        if (useTrash)
        {
            if (fileInfo.isDir())
            {
                if (!DTrash::deleteDirRecursivley(deleteUrl.toLocalFile(), m_data->jobTime()))
                {
                    Q_EMIT signalError(i18n("Could not move folder %1 to collection trash.\n"
                                            "Check the file permission on the trash folder "
                                            "\".dtrash\" in the image collection.",
                                            QDir::toNativeSeparators(fileInfo.path())));

                    continue;
                }
            }
            else
            {
                if (!DTrash::deleteImage(deleteUrl.toLocalFile(), m_data->jobTime()))
                {
                    Q_EMIT signalError(i18n("Could not move image %1 to collection trash.\n"
                                            "Check the file permission on the trash folder "
                                            "\".dtrash\" in the image collection.",
                                            QDir::toNativeSeparators(fileInfo.filePath())));

                    continue;
                }
            }
        }
        else
        {
            if (fileInfo.isDir())
            {
                QDir dir(fileInfo.filePath());

                if (!dir.removeRecursively())
                {
                    Q_EMIT signalError(i18n("Album %1 could not be removed",
                                            QDir::toNativeSeparators(fileInfo.path())));

                    continue;
                }
            }
            else
            {
                QFile file(fileInfo.filePath());

                if (!file.remove())
                {
                    Q_EMIT signalError(i18n("Image %1 could not be removed",
                                            QDir::toNativeSeparators(fileInfo.filePath())));

                    continue;
                }
            }
        }

        Q_EMIT signalOneProccessed(deleteUrl);
    }

    Q_EMIT signalDone();
}

// --------------------------------------------

RenameFileJob::RenameFileJob(IOJobData* const data)
    : m_data(data)
{
}

void RenameFileJob::run()
{
    while (m_data && !m_cancel)
    {
        QUrl renameUrl = m_data->getNextUrl();

        if (renameUrl.isEmpty())
        {
            break;
        }

        QUrl destUrl = m_data->destUrl(renameUrl);
        QFileInfo fileInfo(destUrl.toLocalFile());

        QDir dir(fileInfo.dir());
        const QStringList& dirList = dir.entryList(QDir::Dirs    |
                                                   QDir::Files   |
                                                   QDir::NoDotAndDotDot);

        if (dirList.contains(fileInfo.fileName()))
        {
            if (m_data->fileConflict() == IOJobData::Overwrite)
            {
                if (!DTrash::deleteImage(destUrl.toLocalFile(), m_data->jobTime()))
                {
                    Q_EMIT signalError(i18n("Could not move image %1 to collection trash",
                                            QDir::toNativeSeparators(destUrl.toLocalFile())));

                    Q_EMIT signalRenameFailed(renameUrl);
                    continue;
                }
            }
            else
            {
                qCDebug(DIGIKAM_IOJOB_LOG) << "File with the same name exists!";
                Q_EMIT signalError(i18n("Image with the same name %1 already there",
                                        QDir::toNativeSeparators(destUrl.toLocalFile())));

                Q_EMIT signalRenameFailed(renameUrl);
                continue;
            }
        }

        qCDebug(DIGIKAM_IOJOB_LOG) << "Trying to rename"
                                   << renameUrl.toLocalFile() << "to"
                                   << destUrl.toLocalFile();

        if (!DFileOperations::renameFile(renameUrl.toLocalFile(), destUrl.toLocalFile()))
        {
            qCDebug(DIGIKAM_IOJOB_LOG) << "File could not be renamed!";
            Q_EMIT signalError(i18n("Image %1 could not be renamed",
                                    QDir::toNativeSeparators(renameUrl.toLocalFile())));

            Q_EMIT signalRenameFailed(renameUrl);
            continue;
        }

        Q_EMIT signalOneProccessed(renameUrl);
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

DTrashItemsListingJob::DTrashItemsListingJob(const QString& collectionPath)
    : m_collectionPath(collectionPath)
{
}

void DTrashItemsListingJob::run()
{
    DTrashItemInfo itemInfo;

    QString collectionTrashFilesPath = m_collectionPath + QLatin1Char('/') + DTrash::TRASH_FOLDER +
                                       QLatin1Char('/') + DTrash::FILES_FOLDER;

    qCDebug(DIGIKAM_IOJOB_LOG) << "Collection trash files path:" << collectionTrashFilesPath;

    QDir filesDir(collectionTrashFilesPath);

    Q_FOREACH (const QFileInfo& fileInfo, filesDir.entryInfoList(QDir::Files))
    {
        qCDebug(DIGIKAM_IOJOB_LOG) << "File in trash:" << fileInfo.filePath();
        itemInfo.trashPath = fileInfo.filePath();

        DTrash::extractJsonForItem(m_collectionPath, fileInfo.baseName(), itemInfo);

        Q_EMIT trashItemInfo(itemInfo);
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

RestoreDTrashItemsJob::RestoreDTrashItemsJob(IOJobData* const data)
    : m_data(data)
{
}

void RestoreDTrashItemsJob::run()
{
    if (!m_data)
    {
        return;
    }

    Q_FOREACH (const DTrashItemInfo& item, m_data->trashItems())
    {
        QUrl srcToRename = QUrl::fromLocalFile(item.collectionPath);
        QUrl newName     = DFileOperations::getUniqueFileUrl(srcToRename);

        QFileInfo fi(item.collectionPath);

        if (!fi.dir().exists())
        {
            fi.dir().mkpath(fi.dir().path());
        }

        if (!QFile::rename(item.trashPath, newName.toLocalFile()))
        {
            Q_EMIT signalError(i18n("Could not restore file %1 from trash",
                                    QDir::toNativeSeparators(newName.toLocalFile())));
        }
        else
        {
            QFile::remove(item.jsonFilePath);
        }

        Q_EMIT signalOneProccessed(newName);
    }

    Q_EMIT signalDone();
}

// ----------------------------------------------

EmptyDTrashItemsJob::EmptyDTrashItemsJob(IOJobData* const data)
    : m_data(data)
{
}

void EmptyDTrashItemsJob::run()
{
    if (!m_data)
    {
        return;
    }

    QList<int> albumsFromImages;
    QList<qlonglong> imagesToRemove;

    Q_FOREACH (const DTrashItemInfo& item, m_data->trashItems())
    {
        QFile::remove(item.trashPath);
        QFile::remove(item.jsonFilePath);

        imagesToRemove   << item.imageId;
        albumsFromImages << ItemInfo(item.imageId).albumId();

        Q_EMIT signalOneProccessed(QUrl());
    }

    {
        CoreDbOperationGroup group;
        group.setMaximumTime(200);

        CoreDbAccess().db()->removeItemsPermanently(imagesToRemove, albumsFromImages);
    }

    Q_EMIT signalDone();
}

} // namespace Digikam
