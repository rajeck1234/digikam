/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : contextmenu helper class - Private container.
 *
 * SPDX-FileCopyrightText: 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CONTEXT_MENU_HELPER_P_H
#define DIGIKAM_CONTEXT_MENU_HELPER_P_H

#include "contextmenuhelper.h"

// Qt includes

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QIcon>
#include <QMap>
#include <QMenu>
#include <QMimeData>
#include <QPointer>
#include <QString>
#include <QTimer>

// KDE includes

// Pragma directives to reduce warnings from KDE header files.
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic push
#   pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wmismatched-tags"
#   pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include <kactioncollection.h>
#include <klocalizedstring.h>

#ifdef HAVE_KIO
#   include <kopenwithdialog.h>
#endif

// Restore warnings
#if defined(Q_CC_GNU)
#   pragma GCC diagnostic pop
#endif

#if defined(Q_CC_CLANG)
#   pragma clang diagnostic pop
#endif

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "coredb.h"
#include "albummanager.h"
#include "albumpointer.h"
#include "albummodificationhelper.h"
#include "abstractalbummodel.h"
#include "coredbaccess.h"
#include "digikamapp.h"
#include "dservicemenu.h"
#include "dfileoperations.h"
#include "iteminfo.h"
#include "itemfiltermodel.h"
#include "itemviewutilities.h"
#include "lighttablewindow.h"
#include "queuemgrwindow.h"
#include "picklabelwidget.h"
#include "colorlabelwidget.h"
#include "ratingwidget.h"
#include "tagmodificationhelper.h"
#include "tagspopupmenu.h"
#include "fileactionmngr.h"
#include "tagscache.h"
#include "dimg.h"
#include "dxmlguiwindow.h"

#ifdef HAVE_AKONADICONTACT
#   include "akonadiiface.h"
#endif

#ifdef Q_OS_WIN
#   include <windows.h>
#   include <shellapi.h>
#endif

namespace Digikam
{

class Q_DECL_HIDDEN ContextMenuHelper::Private
{
public:

    explicit Private(ContextMenuHelper* const q)
      : gotoAlbumAction     (nullptr),
        gotoDateAction      (nullptr),
        setThumbnailAction  (nullptr),
        imageFilterModel    (nullptr),
        albumModel          (nullptr),
        parent              (nullptr),
        stdActionCollection (nullptr),
        q                   (q)
    {
    }

    QAction*                     gotoAlbumAction;
    QAction*                     gotoDateAction;
    QAction*                     setThumbnailAction;

    QList<qlonglong>             selectedIds;
    QList<QUrl>                  selectedItems;

    QMap<int, QAction*>          queueActions;
    QMap<QString, KService::Ptr> servicesMap;

    ItemFilterModel*             imageFilterModel;
    AbstractCheckableAlbumModel* albumModel;

    QMenu*                       parent;

    KActionCollection*           stdActionCollection;

    ContextMenuHelper*           q;

public:

    QModelIndex indexForAlbumFromAction(QObject* const sender) const
    {
        QAction* action = nullptr;

        if ((action = qobject_cast<QAction*>(sender)))
        {
            Album* const album = action->data().value<AlbumPointer<> >();

            return albumModel->indexForAlbum(album);
        }

        return QModelIndex();
    }

    QAction* copyFromMainCollection(const QString& name) const
    {
        QAction* const mainAction = stdActionCollection->action(name);

        if (!mainAction)
        {
            return nullptr;
        }

        QAction* const action = new QAction(mainAction->icon(), mainAction->text(), q);
        action->setShortcut(mainAction->shortcut());
        action->setToolTip(mainAction->toolTip());

        return action;
    }
};

} // namespace Digikam

#endif // DIGIKAM_CONTEXT_MENU_HELPER_P_H
