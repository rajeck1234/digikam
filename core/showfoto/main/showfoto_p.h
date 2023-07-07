/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-11-22
 * Description : stand alone digiKam image editor GUI
 *
 * SPDX-FileCopyrightText: 2004-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOW_FOTO_PRIVATE_H
#define SHOW_FOTO_PRIVATE_H

#include "showfoto.h"

// Qt includes

#include <QCursor>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayout>
#include <QList>
#include <QPointer>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QStandardPaths>
#include <QStyle>
#include <QKeySequence>
#include <QMenu>
#include <QMenuBar>
#include <QMimeDatabase>
#include <QMessageBox>
#include <QApplication>
#include <QMimeType>
#include <QSplitter>
#include <QAction>
#include <QUrl>
#include <QScopedPointer>

// KDE includes

#include <kactioncollection.h>
#include <klocalizedstring.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

// Local includes

#include "showfotoiteminfo.h"
#include "showfotothumbnailbar.h"
#include "dsplashscreen.h"
#include "itempropertiessidebar.h"
#include "showfotodragdrophandler.h"
#include "thumbnailloadthread.h"
#include "drawdecoder.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "canvas.h"
#include "editorcore.h"
#include "dmetadata.h"
#include "editorstackview.h"
#include "dfileoperations.h"
#include "iccsettingscontainer.h"
#include "imagedialog.h"
#include "iofilesettings.h"
#include "loadingcache.h"
#include "loadingcacheinterface.h"
#include "metaenginesettings.h"
#include "savingcontext.h"
#include "showfotosetup.h"
#include "showfotosetupmisc.h"
#include "setupicc.h"
#include "statusprogressbar.h"
#include "thememanager.h"
#include "thumbnailsize.h"
#include "dnotificationwrapper.h"
#include "showfotodelegate.h"
#include "showfotothumbnailmodel.h"
#include "showfotocategorizedview.h"
#include "showfotofolderviewsidebar.h"
#include "showfotofolderviewlist.h"
#include "showfotostackviewsidebar.h"
#include "showfotostackviewlist.h"
#include "showfotosettings.h"
#include "showfotoinfoiface.h"
#include "dexpanderbox.h"
#include "dfiledialog.h"
#include "dpluginloader.h"
#include "exiftoolthread.h"
#include "exiftoolparser.h"
#include "sidebarwidget.h"
#include "localizesettings.h"
#include "networkmanager.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN Showfoto::Private
{
public:

    explicit Private()
      : validIccPath            (true),
        itemsNb                 (0),
        vSplitter               (nullptr),
        fileOpenAction          (nullptr),
        openFilesInFolderAction (nullptr),
        mediaServerAction       (nullptr),
        first                   (nullptr),
        model                   (nullptr),
        dDHandler               (nullptr),
        filterModel             (nullptr),
        thumbLoadThread         (nullptr),
        thumbBar                (nullptr),
        thumbBarDock            (nullptr),
        normalDelegate          (nullptr),
        leftSideBar             (nullptr),
        rightSideBar            (nullptr),
        splash                  (nullptr),
        settings                (nullptr),
        folderView              (nullptr),
        stackView               (nullptr)
    {
    }

    bool                              validIccPath;

    int                               itemsNb;

    QSplitter*                        vSplitter;
    QAction*                          fileOpenAction;
    QUrl                              currentLoadedUrl;
    QUrl                              lastOpenedDirectory;
    QAction*                          openFilesInFolderAction;
    QAction*                          mediaServerAction;
    QAction*                          first;

    ShowfotoItemInfoList              infoList;
    ShowfotoThumbnailModel*           model;
    ShowfotoDragDropHandler*          dDHandler;
    ShowfotoFilterModel*              filterModel;
    Digikam::ThumbnailLoadThread*     thumbLoadThread;
    ShowfotoThumbnailBar*             thumbBar;
    Digikam::ThumbBarDock*            thumbBarDock;
    ShowfotoNormalDelegate*           normalDelegate;
    Digikam::Sidebar*                 leftSideBar;
    Digikam::ItemPropertiesSideBar*   rightSideBar;
    Digikam::DSplashScreen*           splash;
    ShowfotoSettings*                 settings;
    ShowfotoFolderViewSideBar*        folderView;
    ShowfotoStackViewSideBar*         stackView;
};

} // namespace ShowFoto

#endif // SHOW_FOTO_PRIVATE_H
