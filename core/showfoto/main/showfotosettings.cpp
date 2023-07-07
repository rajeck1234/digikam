/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-12-20
 * Description : Settings for Showfoto
 *
 * SPDX-FileCopyrightText: 2013-2014 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "showfotosettings.h"

// Qt includes

#include <QStyle>
#include <QApplication>
#include <QFontDatabase>

// KDE includes

#include <ksharedconfig.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_config.h"
#include "digikam_debug.h"
#include "setupmisc.h"
#include "thememanager.h"

namespace ShowFoto
{

class Q_DECL_HIDDEN ShowfotoSettings::Private
{

public:

    explicit Private()
      : deleteItem2Trash        (true),
        showFormatOverThumbnail (false),
        showCoordinates         (false),
        showSplash              (true),
        nativeFileDialog        (false),
        itemCenter              (false),
        reverseSort             (false),
        showToolTip             (true),
        showFileName            (true),
        showFileDate            (false),
        showFileSize            (false),
        showFileType            (false),
        showFileDim             (true),
        showPhotoMake           (true),
        showPhotoLens           (true),
        showPhotoFocal          (true),
        showPhotoExpo           (true),
        showPhotoFlash          (false),
        showPhotoWB             (false),
        showPhotoDate           (true),
        showPhotoMode           (false),
        updateType              (0),
        updateWithDebug         (false),
        rightSideBarStyle       (0),
        sortOrder               (0)
    {
    }

    static const QString configGroupDefault;

    static const QString configLastOpenedDir;
    static const QString configDeleteItem2Trash;
    static const QString configCurrentTheme;
    static const QString configRightSideBarStyle;
    static const QString configUpdateType;
    static const QString configUpdateWithDebug;
    static const QString configApplicationStyle;
    static const QString configIconTheme;
    static const QString configApplicationFont;
    static const QString configShowFormatOverThumbnail;
    static const QString configShowCoordinates;
    static const QString configShowSplash;
    static const QString configNativeFileDialog;
    static const QString configItemCenter;
    static const QString configSortOrder;
    static const QString configReverseSort;

    static const QString configShowToolTip;

    static const QString configShowFileName;
    static const QString configShowFileDate;
    static const QString configShowFileSize;
    static const QString configShowFileType;
    static const QString configShowFileDim;

    static const QString configShowPhotoMake;
    static const QString configShowPhotoLens;
    static const QString configShowPhotoFocal;
    static const QString configShowPhotoExpo;
    static const QString configShowPhotoFlash;
    static const QString configShowPhotoWB;
    static const QString configShowPhotoDate;
    static const QString configShowPhotoMode;

    static const QString configToolTipsFont;

    bool                 deleteItem2Trash;
    bool                 showFormatOverThumbnail;
    bool                 showCoordinates;
    bool                 showSplash;
    bool                 nativeFileDialog;
    bool                 itemCenter;
    bool                 reverseSort;

    bool                 showToolTip;

    bool                 showFileName;
    bool                 showFileDate;
    bool                 showFileSize;
    bool                 showFileType;
    bool                 showFileDim;

    bool                 showPhotoMake;
    bool                 showPhotoLens;
    bool                 showPhotoFocal;
    bool                 showPhotoExpo;
    bool                 showPhotoFlash;
    bool                 showPhotoWB;
    bool                 showPhotoDate;
    bool                 showPhotoMode;

    int                  updateType;
    bool                 updateWithDebug;
    int                  rightSideBarStyle;
    int                  sortOrder;

    QFont                toolTipsFont;

    QString              lastOpenedDir;
    QString              theme;
    QString              applicationStyle;
    QString              applicationIcon;
    QFont                applicationFont;

    KSharedConfigPtr     config;
    KConfigGroup         group;
};

/// Configuration Group
const QString ShowfotoSettings::Private::configGroupDefault(QLatin1String("ImageViewer Settings"));

/// Misc. & Showfoto Generals Settings
const QString ShowfotoSettings::Private::configLastOpenedDir(QLatin1String("Last Opened Directory"));
const QString ShowfotoSettings::Private::configDeleteItem2Trash(QLatin1String("DeleteItem2Trash"));
const QString ShowfotoSettings::Private::configCurrentTheme(QLatin1String("Theme"));
const QString ShowfotoSettings::Private::configUpdateType(QLatin1String("Update Type"));
const QString ShowfotoSettings::Private::configUpdateWithDebug(QLatin1String("Update With Debug"));
const QString ShowfotoSettings::Private::configRightSideBarStyle(QLatin1String("Sidebar Title Style"));
const QString ShowfotoSettings::Private::configApplicationStyle(QLatin1String("Application Style"));
const QString ShowfotoSettings::Private::configIconTheme(QLatin1String("Icon Theme"));
const QString ShowfotoSettings::Private::configApplicationFont(QLatin1String("Application Font"));
const QString ShowfotoSettings::Private::configShowFormatOverThumbnail(QLatin1String("ShowMimeOverImage"));
const QString ShowfotoSettings::Private::configShowCoordinates(QLatin1String("Show Coordinates"));
const QString ShowfotoSettings::Private::configShowSplash(QLatin1String("ShowSplash"));
const QString ShowfotoSettings::Private::configNativeFileDialog(QLatin1String("Use Native File Dialog"));
const QString ShowfotoSettings::Private::configItemCenter(QLatin1String("Item To Center"));
const QString ShowfotoSettings::Private::configSortOrder(QLatin1String("SortOrder"));
const QString ShowfotoSettings::Private::configReverseSort(QLatin1String("ReverseSort"));

/// Tool Tip Enable/Disable
const QString ShowfotoSettings::Private::configShowToolTip(QLatin1String("Show ToolTips"));

/// Tool Tip File Properties
const QString ShowfotoSettings::Private::configShowFileName(QLatin1String("ToolTips Show File Name"));
const QString ShowfotoSettings::Private::configShowFileDate(QLatin1String("ToolTips Show File Date"));
const QString ShowfotoSettings::Private::configShowFileSize(QLatin1String("ToolTips Show File Size"));
const QString ShowfotoSettings::Private::configShowFileType(QLatin1String("ToolTips Show Image Type"));
const QString ShowfotoSettings::Private::configShowFileDim(QLatin1String("ToolTips Show Image Dim"));

/// Tool Tip Photograph Info
const QString ShowfotoSettings::Private::configShowPhotoMake(QLatin1String("ToolTips Show Photo Make"));
const QString ShowfotoSettings::Private::configShowPhotoLens(QLatin1String("ToolTips Show Photo Lens"));
const QString ShowfotoSettings::Private::configShowPhotoFocal(QLatin1String("ToolTips Show Photo Focal"));
const QString ShowfotoSettings::Private::configShowPhotoExpo(QLatin1String("ToolTips Show Photo Expo"));
const QString ShowfotoSettings::Private::configShowPhotoFlash(QLatin1String("ToolTips Show Photo Flash"));
const QString ShowfotoSettings::Private::configShowPhotoWB(QLatin1String("ToolTips Show Photo WB"));
const QString ShowfotoSettings::Private::configShowPhotoDate(QLatin1String("ToolTips Show Photo Date"));
const QString ShowfotoSettings::Private::configShowPhotoMode(QLatin1String("ToolTips Show Photo Mode"));

/// Tool Tips Font
const QString ShowfotoSettings::Private::configToolTipsFont(QLatin1String("ToolTips Font"));

// -------------------------------------------------------------------------------------------------

class Q_DECL_HIDDEN ShowfotoSettingsCreator
{
public:

    ShowfotoSettings object;
};

Q_GLOBAL_STATIC(ShowfotoSettingsCreator, creator)

// -------------------------------------------------------------------------------------------------

ShowfotoSettings* ShowfotoSettings::instance()
{
    return &creator->object;
}

ShowfotoSettings::ShowfotoSettings()
    : QObject(),
      d      (new Private)
{
    d->config = KSharedConfig::openConfig();
    d->group  = d->config->group(d->configGroupDefault);
    init();
    readSettings();
}

ShowfotoSettings::~ShowfotoSettings()
{
    delete d;
}

void ShowfotoSettings::init()
{
    d->updateType              = 0;
    d->updateWithDebug         = false;
    d->rightSideBarStyle       = 0;
    d->sortOrder               = 0;
    d->deleteItem2Trash        = true;
    d->showSplash              = true;
    d->reverseSort             = false;

    d->showFormatOverThumbnail = false;
    d->showCoordinates         = false;
    d->nativeFileDialog        = false;
    d->itemCenter              = false;

    d->showToolTip             = true;

    d->showFileName            = true;
    d->showFileDate            = false;
    d->showFileSize            = false;
    d->showFileType            = false;
    d->showFileDim             = true;

    d->showPhotoMake           = true;
    d->showPhotoLens           = true;
    d->showPhotoFocal          = true;
    d->showPhotoExpo           = true;
    d->showPhotoFlash          = false;
    d->showPhotoWB             = false;
    d->showPhotoDate           = true;
    d->showPhotoMode           = true;

    d->applicationStyle        = qApp->style()->objectName();
    d->applicationIcon         = QString();
    d->applicationFont         = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
}

void ShowfotoSettings::readSettings()
{
    KConfigGroup group         = d->group;

    d->lastOpenedDir           = group.readEntry(d->configLastOpenedDir,           QString());
    d->deleteItem2Trash        = group.readEntry(d->configDeleteItem2Trash,        true);
    d->theme                   = group.readEntry(d->configCurrentTheme,            Digikam::ThemeManager::instance()->defaultThemeName());
    d->updateType              = group.readEntry(d->configUpdateType,              0);
    d->updateWithDebug         = group.readEntry(d->configUpdateWithDebug,         false);
    d->rightSideBarStyle       = group.readEntry(d->configRightSideBarStyle,       0);

#ifdef Q_OS_WIN

    QString defaultStyle       = QLatin1String("Fusion");

#else

    QString defaultStyle       = qApp->style()->objectName();

#endif

#ifdef HAVE_APPSTYLE_SUPPORT

    setApplicationStyle(group.readEntry(d->configApplicationStyle, defaultStyle));

#else

    setApplicationStyle(QLatin1String("Fusion"));

#endif

    d->applicationIcon         = group.readEntry(d->configIconTheme,               QString());

    setApplicationFont(group.readEntry(d->configApplicationFont, QFontDatabase::systemFont(QFontDatabase::GeneralFont)));

    d->showSplash              = group.readEntry(d->configShowSplash,              true);

#ifdef Q_OS_MACOS

    d->nativeFileDialog        = group.readEntry(d->configNativeFileDialog,        true);

#else

    d->nativeFileDialog        = group.readEntry(d->configNativeFileDialog,        false);

#endif

    d->itemCenter              = group.readEntry(d->configItemCenter,              false);
    d->sortOrder               = group.readEntry(d->configSortOrder,               0);
    d->reverseSort             = group.readEntry(d->configReverseSort,             false);
    d->showFormatOverThumbnail = group.readEntry(d->configShowFormatOverThumbnail, false);
    d->showCoordinates         = group.readEntry(d->configShowCoordinates,         false);

    d->showToolTip             = group.readEntry(d->configShowToolTip,             true);

    d->showFileName            = group.readEntry(d->configShowFileName,            true);
    d->showFileDate            = group.readEntry(d->configShowFileDate,            false);
    d->showFileSize            = group.readEntry(d->configShowFileSize,            false);
    d->showFileType            = group.readEntry(d->configShowFileType,            false);
    d->showFileDim             = group.readEntry(d->configShowFileDim,             true);

    d->showPhotoMake           = group.readEntry(d->configShowPhotoMake,           true);
    d->showPhotoLens           = group.readEntry(d->configShowPhotoLens,           true);
    d->showPhotoFocal          = group.readEntry(d->configShowPhotoFocal,          true);
    d->showPhotoExpo           = group.readEntry(d->configShowPhotoExpo,           true);
    d->showPhotoFlash          = group.readEntry(d->configShowPhotoFlash,          false);
    d->showPhotoWB             = group.readEntry(d->configShowPhotoWB,             false);
    d->showPhotoDate           = group.readEntry(d->configShowPhotoDate,           true);
    d->showPhotoMode           = group.readEntry(d->configShowPhotoMode,           true);

    d->toolTipsFont            = group.readEntry(d->configToolTipsFont,            QFontDatabase::systemFont(QFontDatabase::GeneralFont));
}

QString ShowfotoSettings::getLastOpenedDir() const
{
    return d->lastOpenedDir;
}

bool ShowfotoSettings::getDeleteItem2Trash() const
{
    return d->deleteItem2Trash;
}

QString ShowfotoSettings::getCurrentTheme() const
{
    return d->theme;
}

int ShowfotoSettings::getUpdateType() const
{
    return d->updateType;
}

bool ShowfotoSettings::getUpdateWithDebug() const
{
    return d->updateWithDebug;
}

int ShowfotoSettings::getRightSideBarStyle() const
{
    return d->rightSideBarStyle;
}

bool ShowfotoSettings::getShowFormatOverThumbnail() const
{
    return d->showFormatOverThumbnail;
}

bool ShowfotoSettings::getShowCoordinates() const
{
    return d->showCoordinates;
}

QString ShowfotoSettings::getApplicationStyle() const
{
    return d->applicationStyle;
}

QString ShowfotoSettings::getIconTheme() const
{
    return d->applicationIcon;
}

QFont ShowfotoSettings::getApplicationFont() const
{
    return d->applicationFont;
}

bool ShowfotoSettings::getShowSplash() const
{
    return d->showSplash;
}

bool ShowfotoSettings::getNativeFileDialog() const
{
    return d->nativeFileDialog;
}

bool ShowfotoSettings::getItemCenter() const
{
    return d->itemCenter;
}

int ShowfotoSettings::getSortRole() const
{
    return d->sortOrder;
}

bool ShowfotoSettings::getReverseSort() const
{
    return d->reverseSort;
}

bool ShowfotoSettings::getShowToolTip() const
{
    return d->showToolTip;
}

bool ShowfotoSettings::getShowFileName() const
{
    return d->showFileName;
}

bool ShowfotoSettings::getShowFileDate() const
{
    return d->showFileDate;
}

bool ShowfotoSettings::getShowFileSize() const
{
    return d->showFileSize;
}

bool ShowfotoSettings::getShowFileType() const
{
    return d->showFileType;
}

bool ShowfotoSettings::getShowFileDim() const
{
    return d->showFileDim;
}

bool ShowfotoSettings::getShowPhotoMake() const
{
    return d->showPhotoMake;
}

bool ShowfotoSettings::getShowPhotoLens() const
{
    return d->showPhotoLens;
}

bool ShowfotoSettings::getShowPhotoFocal() const
{
    return d->showPhotoFocal;
}

bool ShowfotoSettings::getShowPhotoExpo() const
{
    return d->showPhotoExpo;
}

bool ShowfotoSettings::getShowPhotoFlash() const
{
    return d->showPhotoFlash;
}

bool ShowfotoSettings::getShowPhotoWB() const
{
    return d->showPhotoWB;
}

bool ShowfotoSettings::getShowPhotoDate() const
{
    return d->showPhotoDate;
}

bool ShowfotoSettings::getShowPhotoMode() const
{
    return d->showPhotoMode;
}

QFont ShowfotoSettings::getToolTipFont() const
{
    return d->toolTipsFont;
}

void ShowfotoSettings::setShowToolTip(bool show)
{
    d->group.writeEntry(d->configShowToolTip, show);
}

void ShowfotoSettings::setShowFileName(bool show)
{
    d->group.writeEntry(d->configShowFileName, show);
}

void ShowfotoSettings::setShowFileDate(bool show)
{
    d->group.writeEntry(d->configShowFileDate, show);
}

void ShowfotoSettings::setShowFileSize(bool show)
{
    d->group.writeEntry(d->configShowFileSize, show);
}

void ShowfotoSettings::setShowFileType(bool show)
{
    d->group.writeEntry(d->configShowFileType, show);
}

void ShowfotoSettings::setShowFileDim(bool show)
{
    d->group.writeEntry(d->configShowFileDim, show);
}

void ShowfotoSettings::setShowPhotoMake(bool show)
{
    d->group.writeEntry(d->configShowPhotoMake, show);
}

void ShowfotoSettings::setShowPhotoLens(bool show)
{
    d->group.writeEntry(d->configShowPhotoLens, show);
}

void ShowfotoSettings::setShowPhotoFocal(bool show)
{
    d->group.writeEntry(d->configShowPhotoFocal, show);
}

void ShowfotoSettings::setShowPhotoExpo(bool show)
{
    d->group.writeEntry(d->configShowPhotoExpo, show);
}

void ShowfotoSettings::setShowPhotoFlash(bool show)
{
    d->group.writeEntry(d->configShowPhotoFlash, show);
}

void ShowfotoSettings::setShowPhotoWB(bool show)
{
    d->group.writeEntry(d->configShowPhotoWB, show);
}

void ShowfotoSettings::setShowPhotoDate(bool show)
{
    d->group.writeEntry(d->configShowPhotoDate, show);
}

void ShowfotoSettings::setShowPhotoMode(bool show)
{
    d->group.writeEntry(d->configShowPhotoMode, show);
}

void ShowfotoSettings::setToolTipFont(const QFont& font)
{
    d->group.writeEntry(d->configToolTipsFont, font);
}

void ShowfotoSettings::setLastOpenedDir(const QString& dir)
{
    d->group.writeEntry(d->configLastOpenedDir, dir);
}

void ShowfotoSettings::setDeleteItem2Trash(bool D2t)
{
    d->group.writeEntry(d->configDeleteItem2Trash, D2t);
}

void ShowfotoSettings::setCurrentTheme(const QString& theme)
{
    d->group.writeEntry(d->configCurrentTheme, theme);
}

void ShowfotoSettings::setUpdateType(int type)
{
    d->group.writeEntry(d->configUpdateType, type);
}

void ShowfotoSettings::setUpdateWithDebug(bool dbg)
{
    d->group.writeEntry(d->configUpdateWithDebug, dbg);
}

void ShowfotoSettings::setRightSideBarStyle(int style)
{
    d->group.writeEntry(d->configRightSideBarStyle, style);
}

void ShowfotoSettings::setApplicationStyle(const QString& style)
{
    if (d->applicationStyle.compare(style, Qt::CaseInsensitive) != 0)
    {
        d->applicationStyle = style;
        qApp->setStyle(d->applicationStyle);
        qApp->style()->polish(qApp);
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Switch to widget style: " << d->applicationStyle;
        d->group.writeEntry(d->configApplicationStyle, d->applicationStyle);
    }
}

void ShowfotoSettings::setIconTheme(const QString& theme)
{
    d->group.writeEntry(d->configIconTheme, theme);
}

void ShowfotoSettings::setApplicationFont(const QFont& font)
{
    if (d->applicationFont != font)
    {
        d->applicationFont = font;
        qApp->setFont(d->applicationFont);
        qCDebug(DIGIKAM_SHOWFOTO_LOG) << "Switch to application font: " << d->applicationFont;
        d->group.writeEntry(d->configApplicationFont, d->applicationFont);
    }
}

void ShowfotoSettings::setShowFormatOverThumbnail(bool show)
{
    d->group.writeEntry(d->configShowFormatOverThumbnail, show);
}

void ShowfotoSettings::setShowCoordinates(bool show)
{
    d->group.writeEntry(d->configShowCoordinates, show);
}

void ShowfotoSettings::setShowSplash(bool show)
{
    d->group.writeEntry(d->configShowSplash, show);
}

void ShowfotoSettings::setNativeFileDialog(bool item)
{
    d->group.writeEntry(d->configNativeFileDialog, item);
}

void ShowfotoSettings::setItemCenter(bool item)
{
    d->group.writeEntry(d->configItemCenter, item);
}

void ShowfotoSettings::setSortRole(int order)
{
    d->group.writeEntry(d->configSortOrder, order);
}

void ShowfotoSettings::setReverseSort(bool reverse)
{
    d->group.writeEntry(d->configReverseSort, reverse);
}

void ShowfotoSettings::syncConfig()
{
    d->config->sync();
}

} // namespace Showfoto
