/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-04
 * Description : Various operation on items
 *
 * SPDX-FileCopyrightText: 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2002-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "itemviewutilities.h"

// Qt includes

#include <QStandardPaths>
#include <QStringView>
#include <QFileInfo>
#include <QUrl>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "albumselectdialog.h"
#include "applicationsettings.h"
#include "deletedialog.h"
#include "dfiledialog.h"
#include "dio.h"
#include "iteminfo.h"
#include "imagewindow.h"
#include "lighttablewindow.h"
#include "loadingcacheinterface.h"
#include "queuemgrwindow.h"
#include "thumbnailloadthread.h"
#include "fileactionmngr.h"
#include "dfileoperations.h"
#include "coredb.h"
#include "coredbaccess.h"

namespace Digikam
{

ItemViewUtilities::ItemViewUtilities(QWidget* const parentWidget)
    : QObject (parentWidget),
      m_widget(parentWidget)
{
    connect(this, SIGNAL(signalImagesDeleted(QList<qlonglong>)),
            AlbumManager::instance(), SLOT(slotImagesDeleted(QList<qlonglong>)));
}

void ItemViewUtilities::setAsAlbumThumbnail(Album* album,
                                            const ItemInfo& itemInfo)
{
    if (!album)
    {
        return;
    }

    if      (album->type() == Album::PHYSICAL)
    {
        PAlbum* const palbum = static_cast<PAlbum*>(album);

        QString err;
        AlbumManager::instance()->updatePAlbumIcon(palbum, itemInfo.id(), err);
    }
    else if (album->type() == Album::TAG)
    {
        TAlbum* const talbum = static_cast<TAlbum*>(album);

        QString err;
        AlbumManager::instance()->updateTAlbumIcon(talbum, QString(), itemInfo.id(), err);
    }
}

void ItemViewUtilities::rename(const QUrl& imageUrl,
                               const QString& newName,
                               bool overwrite)
{
    if (imageUrl.isEmpty() || !imageUrl.isLocalFile() || newName.isEmpty())
    {
        return;
    }

    DIO::rename(imageUrl, newName, overwrite);
}

bool ItemViewUtilities::deleteImages(const QList<ItemInfo>& infos,
                                     const DeleteMode deleteMode)
{
    if (infos.isEmpty())
    {
        return false;
    }

    QList<ItemInfo> deleteInfos = infos;

    QList<QUrl> urlList;
    QList<qlonglong> imageIds;

    // Buffer the urls for deletion and imageids for notification of the AlbumManager

    Q_FOREACH (const ItemInfo& info, deleteInfos)
    {
        urlList  << info.fileUrl();
        imageIds << info.id();
    }

    DeleteDialog dialog(m_widget);

    DeleteDialogMode::DeleteMode deleteDialogMode = DeleteDialogMode::NoChoiceTrash;

    if (deleteMode == ItemViewUtilities::DeletePermanently)
    {
        deleteDialogMode = DeleteDialogMode::NoChoiceDeletePermanently;
    }

    if (!dialog.confirmDeleteList(urlList, DeleteDialogMode::Files, deleteDialogMode))
    {
        return false;
    }

    const bool useTrash = !dialog.shouldDelete();

    DIO::del(deleteInfos, useTrash);

    // Signal the Albummanager about the ids of the deleted images.

    Q_EMIT signalImagesDeleted(imageIds);

    return true;
}

void ItemViewUtilities::deleteImagesDirectly(const QList<ItemInfo>& infos,
                                             const DeleteMode deleteMode)
{
    // This method deletes the selected items directly, without confirmation.
    // It is not used in the default setup.

    if (infos.isEmpty())
    {
        return;
    }

    QList<qlonglong> imageIds;

    Q_FOREACH (const ItemInfo& info, infos)
    {
        imageIds << info.id();
    }

    const bool useTrash = (deleteMode == ItemViewUtilities::DeleteUseTrash);

    DIO::del(infos, useTrash);

    // Signal the Albummanager about the ids of the deleted images.

    Q_EMIT signalImagesDeleted(imageIds);
}

void ItemViewUtilities::notifyFileContentChanged(const QList<QUrl>& urls)
{
    Q_FOREACH (const QUrl& url, urls)
    {
        QString path = url.toLocalFile();
        ThumbnailLoadThread::deleteThumbnail(path);

        // clean LoadingCache as well - be pragmatic, do it here.

        LoadingCacheInterface::fileChanged(path);
    }
}

void ItemViewUtilities::copyItemsToExternalFolder(const QList<ItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    KSharedConfig::Ptr config = KSharedConfig::openConfig();
    KConfigGroup group        = config->group(QLatin1String("Copy To Folder Settings"));
    QString startingPath      = group.readEntry(QLatin1String("Last Copy To Folder Path"), QString());

    if (startingPath.isEmpty())
    {
        startingPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    QUrl url = DFileDialog::getExistingDirectoryUrl(m_widget, i18nc("@title:window", "Select Target Folder"),
                                                    QUrl::fromLocalFile(startingPath));

    if (url.isEmpty() || !url.isLocalFile())
    {
        return;
    }

    group.writeEntry(QLatin1String("Last Copy To Folder Path"), url.toLocalFile());

    DIO::copy(infos, url);
}

void ItemViewUtilities::createNewAlbumForInfos(const QList<ItemInfo>& infos,
                                               Album* currentAlbum)
{
    if (infos.isEmpty())
    {
        return;
    }

    if (currentAlbum && (currentAlbum->type() != Album::PHYSICAL))
    {
        currentAlbum = nullptr;
    }

    QString header(i18n("<p>Please select the destination album from the digiKam library to "
                        "move the selected images into.</p>"));

    Album* const album = AlbumSelectDialog::selectAlbum(m_widget, static_cast<PAlbum*>(currentAlbum), header);

    if (!album)
    {
        return;
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(album);

    if (!palbum)
    {
        return;
    }

    DIO::move(infos, palbum);
}

void ItemViewUtilities::insertToLightTableAuto(const QList<ItemInfo>& all,
                                               const QList<ItemInfo>& selected,
                                               const ItemInfo& current)
{
    ItemInfoList list   = ItemInfoList(selected);
    ItemInfo singleInfo = current;

    if (list.isEmpty() || ((list.size() == 1) && LightTableWindow::lightTableWindow()->isEmpty()))
    {
        list = ItemInfoList(all);
    }

    if (singleInfo.isNull() && !list.isEmpty())
    {
        singleInfo = list.first();
    }

    insertToLightTable(list, current, (list.size() <= 1));
}

void ItemViewUtilities::insertToLightTable(const QList<ItemInfo>& list,
                                            const ItemInfo& current,
                                            bool addTo)
{
    LightTableWindow* const ltview = LightTableWindow::lightTableWindow();

    // If addTo is false, the light table will be emptied before adding
    // the images.

    ltview->loadItemInfos(ItemInfoList(list), current, addTo);
    ltview->setLeftRightItems(ItemInfoList(list), addTo);

    if (ltview->isHidden())
    {
        ltview->show();
    }

    ltview->unminimizeAndActivateWindow();
}

void ItemViewUtilities::insertToQueueManager(const QList<ItemInfo>& list, const ItemInfo& current, bool newQueue)
{
    Q_UNUSED(current);

    QueueMgrWindow* const bqmview = QueueMgrWindow::queueManagerWindow();

    if (bqmview->isHidden())
    {
        bqmview->show();
    }

    bqmview->unminimizeAndActivateWindow();

    if (newQueue)
    {
        bqmview->loadItemInfosToNewQueue(ItemInfoList(list));
    }
    else
    {
        bqmview->loadItemInfosToCurrentQueue(ItemInfoList(list));
    }
}

void ItemViewUtilities::insertSilentToQueueManager(const QList<ItemInfo>& list,
                                                   const ItemInfo& /*current*/,
                                                   int queueid)
{
    QueueMgrWindow* const bqmview = QueueMgrWindow::queueManagerWindow();
    bqmview->loadItemInfos(ItemInfoList(list), queueid);
}

void ItemViewUtilities::openInfos(const ItemInfo& info,
                                  const QList<ItemInfo>& allInfosToOpen,
                                  Album* currentAlbum)
{
    if (info.isNull())
    {
        return;
    }

    QFileInfo fi(info.filePath());
    QString imagefilter = ApplicationSettings::instance()->getImageFileFilter();
    imagefilter        += ApplicationSettings::instance()->getRawFileFilter();

    // If the current item is not an image file.

    if (!imagefilter.contains(fi.suffix().toLower()))
    {
        // Openonly the first one from the list.

        openInfosWithDefaultApplication(QList<ItemInfo>() << info);
        return;
    }

    // Run digiKam ImageEditor with all image from current Album.

    ImageWindow* const imview = ImageWindow::imageWindow();

    imview->disconnect(this);

    connect(imview, SIGNAL(signalURLChanged(QUrl)),
            this, SIGNAL(editorCurrentUrlChanged(QUrl)));

    imview->loadItemInfos(ItemInfoList(allInfosToOpen), info,
                          currentAlbum ? i18n("Album \"%1\"", currentAlbum->title())
                                       : QString());

    if (imview->isHidden())
    {
        imview->show();
    }

    imview->unminimizeAndActivateWindow();
}

void ItemViewUtilities::openInfosWithDefaultApplication(const QList<ItemInfo>& infos)
{
    if (infos.isEmpty())
    {
        return;
    }

    QList<QUrl> urls;

    Q_FOREACH (const ItemInfo& inf, infos)
    {
        urls << inf.fileUrl();
    }

    DFileOperations::openFilesWithDefaultApplication(urls);
}

namespace
{

bool lessThanByTimeForItemInfo(const ItemInfo& a, const ItemInfo& b)
{
    return (a.dateTime() < b.dateTime());
}

bool lowerThanByNameForItemInfo(const ItemInfo& a, const ItemInfo& b)
{
    return (a.name() < b.name());
}

bool lowerThanBySizeForItemInfo(const ItemInfo& a, const ItemInfo& b)
{
    return (a.fileSize() < b.fileSize());
}

} // namespace

void ItemViewUtilities::createGroupByTimeFromInfoList(const ItemInfoList& itemInfoList)
{
    QList<ItemInfo> groupingList = itemInfoList;

    // sort by time

    std::stable_sort(groupingList.begin(), groupingList.end(), lessThanByTimeForItemInfo);

    QList<ItemInfo>::iterator it, it2;

    for (it = groupingList.begin() ; it != groupingList.end() ; )
    {
        const ItemInfo& leader = *it;
        QList<ItemInfo> group;
        QDateTime time         = it->dateTime();

        if (time.isValid())
        {
            for (it2 = it + 1 ; it2 != groupingList.end() ; ++it2)
            {
                if (qAbs(time.secsTo(it2->dateTime())) < 2)
                {
                    group << *it2;
                }
                else
                {
                    break;
                }
            }
        }
        else
        {
            ++it;
            continue;
        }

        // increment to next item not put in the group

        it = it2;

        if (!group.isEmpty())
        {
            FileActionMngr::instance()->addToGroup(leader, group);
        }
    }
}

void ItemViewUtilities::createGroupByFilenameFromInfoList(const ItemInfoList& itemInfoList)
{
    QList<ItemInfo> groupingList = itemInfoList;

    // sort by Name

    std::stable_sort(groupingList.begin(), groupingList.end(), lowerThanByNameForItemInfo);

    QList<ItemInfo>::iterator it, it2;

    for (it = groupingList.begin() ; it != groupingList.end() ; )
    {
        QList<ItemInfo> group;
        QString fname = it->name().left(it->name().lastIndexOf(QLatin1Char('.')));

        // don't know the leader yet so put first element also in group

        group << *it;

        for (it2 = it + 1 ; it2 != groupingList.end() ; ++it2)
        {
            QString fname2 = it2->name().left(it2->name().lastIndexOf(QLatin1Char('.')));

            if (fname == fname2)
            {
                group << *it2;
            }
            else
            {
                break;
            }
        }

        // increment to next item not put in the group

        it = it2;

        if (group.count() > 1)
        {
            // sort by filesize and take smallest as leader

            std::stable_sort(group.begin(), group.end(), lowerThanBySizeForItemInfo);
            const ItemInfo& leader = group.takeFirst();
            FileActionMngr::instance()->addToGroup(leader, group);
        }
    }
}

namespace
{

class Q_DECL_HIDDEN NumberInFilenameMatch
{
public:

    NumberInFilenameMatch()
        : value(0),
          containsValue(false)
    {
    }

    explicit NumberInFilenameMatch(const QString& filename)
        : NumberInFilenameMatch()
    {
        if (filename.isEmpty())
        {
            return;
        }

        auto firstDigit = std::find_if(filename.begin(), filename.end(),
                                       [](const QChar& c)
                                           {
                                               return c.isDigit();
                                           }
                                      );

        prefix = QStringView{filename}.left(std::distance(filename.begin(), firstDigit));

        if (firstDigit == filename.end())
        {
            return;
        }

        auto lastDigit = std::find_if(firstDigit, filename.end(),
                                      [](const QChar& c)
                                          {
                                                return !c.isDigit();
                                          }
                                     );

        value  = filename.mid(prefix.size(),
                              std::distance(firstDigit,
                                            lastDigit)).toULongLong(&containsValue);

        suffix = QStringView{filename}.mid(std::distance(lastDigit, filename.end()));
    }

    bool directlyPreceeds(NumberInFilenameMatch const& other) const
    {
        if (!containsValue || !other.containsValue)
        {
            return false;
        }

        if (prefix != other.prefix)
        {
            return false;
        }

        if (suffix != other.suffix)
        {
            return false;
        }

        return ((value + 1) == other.value);
    }

public:

    qulonglong value;
    QStringView prefix;
    QStringView suffix;
    bool       containsValue;
};

bool imageMatchesTimelapseGroup(const ItemInfoList& group, const ItemInfo& itemInfo)
{
    if (group.size() < 2)
    {
        return true;
    }

    auto const timeBetweenPhotos      = qAbs(group.first().dateTime()
                                                          .secsTo(group.last()
                                                          .dateTime())) / (group.size()-1);

    auto const predictedNextTimestamp = group.last().dateTime()
                                                    .addSecs(timeBetweenPhotos);

    return (qAbs(itemInfo.dateTime().secsTo(predictedNextTimestamp)) <= 1);
}

} // namespace

void ItemViewUtilities::createGroupByTimelapseFromInfoList(const ItemInfoList& itemInfoList)
{
    if (itemInfoList.size() < 3)
    {
        return;
    }

    ItemInfoList groupingList = itemInfoList;

    std::stable_sort(groupingList.begin(), groupingList.end(), lowerThanByNameForItemInfo);

    NumberInFilenameMatch previousNumberMatch;
    ItemInfoList group;

    for (const auto& itemInfo : groupingList)
    {
        NumberInFilenameMatch numberMatch(itemInfo.name());

        // if this is an end of currently processed group

        if (!previousNumberMatch.directlyPreceeds(numberMatch) || !imageMatchesTimelapseGroup(group, itemInfo))
        {
            if (group.size() > 2)
            {
                FileActionMngr::instance()->addToGroup(group.takeFirst(), group);
            }

            group.clear();
        }

        group.append(itemInfo);
        previousNumberMatch = std::move(numberMatch);
    }

    if (group.size() > 2)
    {
        FileActionMngr::instance()->addToGroup(group.takeFirst(), group);
    }
}

} // namespace Digikam
