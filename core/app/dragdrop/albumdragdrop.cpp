/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * SPDX-FileCopyrightText: 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumdragdrop.h"

// Qt includes

#include <QDropEvent>
#include <QMenu>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albumpointer.h"
#include "importui.h"
#include "ddragobjects.h"
#include "dio.h"
#include "iteminfo.h"
#include "iteminfolist.h"

namespace Digikam
{

AlbumDragDropHandler::AlbumDragDropHandler(AlbumModel* const model)
    : AlbumModelDragDropHandler(model)
{
}

AlbumModel* AlbumDragDropHandler::model() const
{
    return (static_cast<AlbumModel*>(m_model));
}

bool AlbumDragDropHandler::dropEvent(QAbstractItemView* view,
                                     const QDropEvent* e,
                                     const QModelIndex& droppedOn)
{
    if (accepts(e, droppedOn) == Qt::IgnoreAction)
    {
        return false;
    }

    AlbumPointer<PAlbum> destAlbum = model()->albumForIndex(droppedOn);

    if (!destAlbum)
    {
        return false;
    }

    if (DAlbumDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        int         albumId = 0;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return false;
        }

        AlbumPointer<PAlbum> droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!droppedAlbum)
        {
            return false;
        }

        // TODO Copy?

        QMenu popMenu(view);
        QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")), i18n("&Move Here"));
        popMenu.addSeparator();
        popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* const choice     = popMenu.exec(QCursor::pos());

        if (choice == moveAction)
        {
            DIO::move(droppedAlbum, destAlbum);
        }

        return true;
    }
    else if (DItemDrag::canDecode(e->mimeData()))
    {

        QList<QUrl>      urls;
        QList<int>       albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, albumIDs, imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
        {
            return false;
        }

        // Check if items dropped come from outside current album.
        // This can be the case with recursive content album mode.

        ItemInfoList extImgInfList;

        for (QList<qlonglong>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
        {
            ItemInfo info(*it);

            if (info.albumId() != destAlbum->id())
            {
                extImgInfList << info;
            }
        }

        if (extImgInfList.isEmpty())
        {
            // Setting the dropped image as the album thumbnail
            // If the ctrl key is pressed, when dropping the image, the
            // thumbnail is set without a popup menu

            bool set = false;

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
            if (e->modifiers() == Qt::ControlModifier)
#else
            if (e->keyboardModifiers() == Qt::ControlModifier)
#endif
            {
                set = true;
            }
            else
            {
                QMenu popMenu(view);
                QAction* setAction    = nullptr;

                if (imageIDs.count() == 1)
                {
                    setAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
                }

                popMenu.addSeparator();
                popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
                popMenu.setMouseTracking(true);
                QAction* const choice = popMenu.exec(QCursor::pos());
                set                   = (setAction == choice);
            }

            if (set && destAlbum)
            {
                QString errMsg;
                AlbumManager::instance()->updatePAlbumIcon(destAlbum, imageIDs.first(), errMsg);
            }

            return true;
        }

        bool ddMove       = false;
        bool ddCopy       = false;
        bool setThumbnail = false;

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        if      (e->modifiers() == Qt::ShiftModifier)
#else
        if      (e->keyboardModifiers() == Qt::ShiftModifier)
#endif
        {
            // If shift key is pressed while dragging, move the drag object without
            // displaying popup menu -> move

            ddMove = true;
        }
#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        else if (e->modifiers() == Qt::ControlModifier)
#else
        else if (e->keyboardModifiers() == Qt::ControlModifier)
#endif
        {
            // If ctrl key is pressed while dragging, copy the drag object without
            // displaying popup menu -> copy

            ddCopy = true;
        }
        else
        {
            QMenu popMenu(view);
            QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
            QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
            QAction* thumbnailAction  = nullptr;

            if (imageIDs.count() == 1)
            {
                thumbnailAction = popMenu.addAction(i18n("Set as Album Thumbnail"));
            }

            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* const choice     = popMenu.exec(QCursor::pos());

            if (choice)
            {
                if      (choice == moveAction)
                {
                    ddMove = true;
                }
                else if (choice == copyAction)
                {
                    ddCopy = true;
                }
                else if (choice == thumbnailAction)
                {
                    setThumbnail = true;
                }
            }
        }

        if (!destAlbum)
        {
            return false;
        }

        if      (ddMove)
        {
            DIO::move(extImgInfList, destAlbum);
        }
        else if (ddCopy)
        {
            DIO::copy(extImgInfList, destAlbum);
        }
        else if (setThumbnail)
        {
            QString errMsg;
            AlbumManager::instance()->updatePAlbumIcon(destAlbum, extImgInfList.first().id(), errMsg);
        }

        return true;
    }

    // -- DnD from Camera GUI ----------------------------

    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        ImportUI* const ui = dynamic_cast<ImportUI*>(e->source());

        if (ui)
        {
            QMenu popMenu(view);
            QAction* const downAction    = popMenu.addAction(QIcon::fromTheme(QLatin1String("file-export")), i18n("Download From Camera"));
            QAction* const downDelAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("file-export")), i18n("Download && Delete From Camera"));
            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* const choice         = popMenu.exec(QCursor::pos());

            if (choice)
            {
                if      (choice == downAction)
                {
                    ui->slotDownload(true, false, destAlbum);
                }
                else if (choice == downDelAction)
                {
                    ui->slotDownload(true, true, destAlbum);
                }
            }
        }
    }

    // -- DnD from an external source ---------------------

    else if (e->mimeData()->hasUrls())
    {
        QList<QUrl> srcURLs = e->mimeData()->urls();
        bool ddMove         = false;
        bool ddCopy         = false;

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        if      (e->modifiers() == Qt::ShiftModifier)
#else
        if      (e->keyboardModifiers() == Qt::ShiftModifier)
#endif
        {
            // If shift key is pressed while dropping, move the drag object without
            // displaying popup menu -> move

            ddMove = true;
        }
#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        else if (e->modifiers() == Qt::ControlModifier)
#else
        else if (e->keyboardModifiers() == Qt::ControlModifier)
#endif
        {
            // If ctrl key is pressed while dropping, copy the drag object without
            // displaying popup menu -> copy

            ddCopy = true;
        }
        else
        {
            QMenu popMenu(view);
            QAction* const moveAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("go-jump")),   i18n("&Move Here"));
            QAction* const copyAction = popMenu.addAction(QIcon::fromTheme(QLatin1String("edit-copy")), i18n("&Copy Here"));
            popMenu.addSeparator();
            popMenu.addAction(QIcon::fromTheme(QLatin1String("dialog-cancel")), i18n("C&ancel"));
            popMenu.setMouseTracking(true);
            QAction* const choice     = popMenu.exec(QCursor::pos());

            if      (choice == copyAction)
            {
                ddCopy = true;
            }
            else if (choice == moveAction)
            {
                ddMove = true;
            }
        }

        if      (ddMove)
        {
            DIO::move(srcURLs, destAlbum);
        }
        else if (ddCopy)
        {
            DIO::copy(srcURLs, destAlbum);
        }

        return true;
    }

    return false;
}

Qt::DropAction AlbumDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& dropIndex)
{
    PAlbum* const destAlbum = model()->albumForIndex(dropIndex);

    if (!destAlbum)
    {
        return Qt::IgnoreAction;
    }

    // Dropping on root is not allowed and
    // Dropping on trash is not implemented yet

    if (destAlbum->isRoot() || destAlbum->isTrashAlbum())
    {
        return Qt::IgnoreAction;
    }

    if      (DAlbumDrag::canDecode(e->mimeData()))
    {
        QList<QUrl> urls;
        int         albumId = 0;

        if (!DAlbumDrag::decode(e->mimeData(), urls, albumId))
        {
            return Qt::IgnoreAction;
        }

        PAlbum* const droppedAlbum = AlbumManager::instance()->findPAlbum(albumId);

        if (!droppedAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging an item on itself makes no sense

        if (droppedAlbum == destAlbum)
        {
            return Qt::IgnoreAction;
        }

        // Dragging a parent on its child makes no sense

        if (droppedAlbum->isAncestorOf(destAlbum))
        {
            return Qt::IgnoreAction;
        }

        return Qt::MoveAction;
    }
    else if (DItemDrag::canDecode(e->mimeData())           ||
             DCameraItemListDrag::canDecode(e->mimeData()) ||
             e->mimeData()->hasUrls())
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList AlbumDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;

    mimeTypes << DAlbumDrag::mimeTypes()
              << DItemDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << QLatin1String("text/uri-list");

    return mimeTypes;
}

QMimeData* AlbumDragDropHandler::createMimeData(const QList<Album*>& albums)
{
    if (albums.isEmpty())
    {
        return nullptr;
    }

    if (albums.size() > 1)
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Dragging multiple albums is not implemented";
    }

    PAlbum* const palbum = dynamic_cast<PAlbum*>(albums.first());

    // Root or album root and Trash Albums are not draggable

    if (!palbum || palbum->isRoot() || palbum->isAlbumRoot() || palbum->isTrashAlbum())
    {
        return nullptr;
    }

    return (new DAlbumDrag(albums.first()->databaseUrl(), albums.first()->id(), palbum->fileUrl()));
}

} // namespace Digikam
