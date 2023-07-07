/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-16-10
 * Description : application settings interface
 *
 * SPDX-FileCopyrightText: 2003-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2015      by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 * SPDX-FileCopyrightText: 2017      by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_APPLICATION_SETTINGS_PRIVATE_H
#define DIGIKAM_APPLICATION_SETTINGS_PRIVATE_H

#include "applicationsettings.h"

// C++ includes

#include <stdexcept>

// Qt includes

#include <QApplication>
#include <QFontDatabase>
#include <QString>
#include <QStringList>
#include <QFont>
#include <QCheckBox>
#include <QPointer>
#include <QMessageBox>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>
#include <kconfiggroup.h>

#ifdef HAVE_KFILEMETADATA
#   include "baloowrap.h"
#endif

// Local includes

#include "previewsettings.h"
#include "dbengineparameters.h"
#include "versionmanager.h"
#include "itemfiltersettings.h"
#include "itemsortsettings.h"
#include "mimefilter.h"
#include "thumbnailsize.h"
#include "thememanager.h"
#include "digikam_debug.h"
#include "drawdecoder.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "haariface.h"

namespace Digikam
{

class ApplicationSettings;

class Q_DECL_HIDDEN ApplicationSettings::Private
{
public:

    explicit Private(ApplicationSettings* const qq);
    ~Private();

    void init();

public:

    static const QString configGroupDefault;
    static const QString configGroupExif;
    static const QString configGroupMetadata;
    static const QString configGroupBaloo;
    static const QString configGroupGeneral;
    static const QString configGroupVersioning;
    static const QString configGroupFaceDetection;
    static const QString configGroupDuplicatesSearch;
    static const QString configGroupGrouping;
    static const QString configAlbumCollectionsEntry;
    static const QString configAlbumMonitoringEntry;
    static const QString configAlbumSortRoleEntry;
    static const QString configImageSortOrderEntry;
    static const QString configImageSortingEntry;
    static const QString configImageSeparationModeEntry;
    static const QString configImageSeparationSortOrderEntry;
    static const QString configItemLeftClickActionEntry;
    static const QString configDefaultIconSizeEntry;
    static const QString configDefaultTreeIconSizeEntry;
    static const QString configDefaultTreeFaceSizeEntry;
    static const QString configTreeViewFontEntry;
    static const QString configThemeEntry;
    static const QString configUpdateType;
    static const QString configUpdateWithDebug;
    static const QString configSidebarTitleStyleEntry;
    static const QString configUseNativeFileDialogEntry;
    static const QString configDrawFramesToGroupedEntry;
    static const QString configExpandNewCurrentItemEntry;
    static const QString configScrollItemToCenterEntry;
    static const QString configShowOnlyPersonTagsInPeopleSidebarEntry;
    static const QString configDetectFacesInNewImagesEntry;
    static const QString configRatingFilterConditionEntry;
    static const QString configRecursiveAlbumsEntry;
    static const QString configRecursiveTagsEntry;
    static const QString configAllGroupsOpenEntry;
    static const QString configIconShowNameEntry;
    static const QString configIconShowResolutionEntry;
    static const QString configIconShowSizeEntry;
    static const QString configIconShowDateEntry;
    static const QString configIconShowModificationDateEntry;
    static const QString configIconShowTitleEntry;
    static const QString configIconShowCommentsEntry;
    static const QString configIconShowTagsEntry;
    static const QString configIconShowOverlaysEntry;
    static const QString configIconShowFullscreenEntry;
    static const QString configIconShowRatingEntry;
    static const QString configIconShowPickLabelEntry;
    static const QString configIconShowColorLabelEntry;
    static const QString configIconShowImageFormatEntry;
    static const QString configIconShowCoordinatesEntry;
    static const QString configIconShowAspectRatioEntry;
    static const QString configIconViewFontEntry;
    static const QString configToolTipsFontEntry;
    static const QString configShowToolTipsEntry;
    static const QString configToolTipsShowFileNameEntry;
    static const QString configToolTipsShowFileDateEntry;
    static const QString configToolTipsShowFileSizeEntry;
    static const QString configToolTipsShowImageTypeEntry;
    static const QString configToolTipsShowImageDimEntry;
    static const QString configToolTipsShowImageAREntry;
    static const QString configToolTipsShowPhotoMakeEntry;
    static const QString configToolTipsShowPhotoLensEntry;
    static const QString configToolTipsShowPhotoDateEntry;
    static const QString configToolTipsShowPhotoFocalEntry;
    static const QString configToolTipsShowPhotoExpoEntry;
    static const QString configToolTipsShowPhotoModeEntry;
    static const QString configToolTipsShowPhotoFlashEntry;
    static const QString configToolTipsShowPhotoWBEntry;
    static const QString configToolTipsShowVideoAspectRatioEntry;
    static const QString configToolTipsShowVideoAudioBitRateEntry;
    static const QString configToolTipsShowVideoAudioChannelTypeEntry;
    static const QString configToolTipsShowVideoAudioCodecEntry;
    static const QString configToolTipsShowVideoDurationEntry;
    static const QString configToolTipsShowVideoFrameRateEntry;
    static const QString configToolTipsShowVideoVideoCodecEntry;
    static const QString configToolTipsShowAlbumNameEntry;
    static const QString configToolTipsShowTitlesEntry;
    static const QString configToolTipsShowCommentsEntry;
    static const QString configToolTipsShowTagsEntry;
    static const QString configToolTipsShowLabelRatingEntry;
    static const QString configShowAlbumToolTipsEntry;
    static const QString configToolTipsShowAlbumTitleEntry;
    static const QString configToolTipsShowAlbumDateEntry;
    static const QString configToolTipsShowAlbumCollectionEntry;
    static const QString configToolTipsShowAlbumCategoryEntry;
    static const QString configToolTipsShowAlbumCaptionEntry;
    static const QString configToolTipsShowAlbumPreviewEntry;
    static const QString configPreviewLoadFullItemSizeEntry;
    static const QString configPreviewRawUseLoadingDataEntry;
    static const QString configPreviewConvertToEightBitEntry;
    static const QString configPreviewScaleFitToWindowEntry;
    static const QString configPreviewShowIconsEntry;
    static const QString configShowThumbbarEntry;
    static const QString configShowFolderTreeViewItemsCountEntry;
    static const QString configShowSplashEntry;
    static const QString configUseTrashEntry;
    static const QString configShowTrashDeleteDialogEntry;
    static const QString configShowPermanentDeleteDialogEntry;
    static const QString configApplySidebarChangesDirectlyEntry;
    static const QString configScanAtStartEntry;
    static const QString configCleanAtStartEntry;
    static const QString configSyncBalootoDigikamEntry;
    static const QString configSyncDigikamtoBalooEntry;
    static const QString configStringComparisonTypeEntry;
    static const QString configFaceDetectionAccuracyEntry;
    static const QString configFaceDetectionYoloV3Entry;
    static const QString configApplicationStyleEntry;
    static const QString configIconThemeEntry;
    static const QString configApplicationFontEntry;
    static const QString configMinimumSimilarityBound;
    static const QString configDuplicatesSearchLastMinSimilarity;
    static const QString configDuplicatesSearchLastMaxSimilarity;
    static const QString configDuplicatesSearchLastAlbumTagRelation;
    static const QString configDuplicatesSearchLastRestrictions;
    static const QString configDuplicatesSearchReferenceSelectionMethod;
    static const ApplicationSettings::OperationStrings configGroupingOperateOnAll;

    /// start up setting
    bool                                         showSplash;
    /// file ops settings
    bool                                         useTrash;
    bool                                         showTrashDeleteDialog;
    bool                                         showPermanentDeleteDialog;
    /// metadata setting
    bool                                         sidebarApplyDirectly;
    /// file dialog setting
    bool                                         useNativeFileDialog;
    /// grouped item draw setting
    bool                                         drawFramesToGrouped;
    /// expand item setting
    bool                                         expandNewCurrentItem;
    /// item center setting
    bool                                         scrollItemToCenter;
    /// tag filter setting
    bool                                         showOnlyPersonTagsInPeopleSidebar;

    bool                                         detectFacesInNewImages;

    /// icon view settings
    bool                                         iconShowName;
    bool                                         iconShowSize;
    bool                                         iconShowDate;
    bool                                         iconShowModDate;
    bool                                         iconShowTitle;
    bool                                         iconShowComments;
    bool                                         iconShowResolution;
    bool                                         iconShowTags;
    bool                                         iconShowOverlays;
    bool                                         iconShowFullscreen;
    bool                                         iconShowRating;
    bool                                         iconShowPickLabel;
    bool                                         iconShowColorLabel;
    bool                                         iconShowImageFormat;
    bool                                         iconShowCoordinates;
    bool                                         iconShowAspectRatio;
    QFont                                        iconviewFont;

    /// Icon-view tooltip settings
    bool                                         showToolTips;
    bool                                         tooltipShowFileName;
    bool                                         tooltipShowFileDate;
    bool                                         tooltipShowFileSize;
    bool                                         tooltipShowImageType;
    bool                                         tooltipShowImageDim;
    bool                                         tooltipShowImageAR;
    bool                                         tooltipShowPhotoMake;
    bool                                         tooltipShowPhotoLens;
    bool                                         tooltipShowPhotoDate;
    bool                                         tooltipShowPhotoFocal;
    bool                                         tooltipShowPhotoExpo;
    bool                                         tooltipShowPhotoMode;
    bool                                         tooltipShowPhotoFlash;
    bool                                         tooltipShowPhotoWb;
    bool                                         tooltipShowAlbumName;
    bool                                         tooltipShowTitles;
    bool                                         tooltipShowComments;
    bool                                         tooltipShowTags;
    bool                                         tooltipShowLabelRating;
    bool                                         tooltipShowVideoAspectRatio;
    bool                                         tooltipShowVideoAudioBitRate;
    bool                                         tooltipShowVideoAudioChannelType;
    bool                                         tooltipShowVideoAudioCodec;
    bool                                         tooltipShowVideoDuration;
    bool                                         tooltipShowVideoFrameRate;
    bool                                         tooltipShowVideoVideoCodec;

    QFont                                        toolTipsFont;

    /// Folder-view tooltip settings
    bool                                         showAlbumToolTips;
    bool                                         tooltipShowAlbumTitle;
    bool                                         tooltipShowAlbumDate;
    bool                                         tooltipShowAlbumCollection;
    bool                                         tooltipShowAlbumCategory;
    bool                                         tooltipShowAlbumCaption;
    bool                                         tooltipShowAlbumPreview;

    /// preview settings
    PreviewSettings                              previewSettings;
    bool                                         scaleFitToWindow;
    bool                                         previewShowIcons;
    bool                                         showThumbbar;

    bool                                         showFolderTreeViewItemsCount;

    /// tree-view settings
    int                                          treeThumbnailSize;
    int                                          treeThumbFaceSize;
    QFont                                        treeviewFont;

    /// icon view settings
    int                                          thumbnailSize;
    int                                          ratingFilterCond;
    bool                                         recursiveAlbums;
    bool                                         recursiveTags;
    bool                                         allGroupsOpen;

    /// theme settings
    QString                                      currentTheme;

    /// database settings
    DbEngineParameters                           databaseParams;
    bool                                         scanAtStart;
    bool                                         cleanAtStart;
    bool                                         databaseDirSetAtCmd;

    /// album settings
    bool                                         albumMonitoring;

    QStringList                                  albumCategoryNames;

    KSharedConfigPtr                             config;

    int                                          updateType;
    bool                                         updateWithDebug;
    DMultiTabBar::TextStyle                      sidebarTitleStyle;

    /// album view settings
    ApplicationSettings::AlbumSortRole           albumSortRole;
    bool                                         albumSortChanged;

    /// icon view settings
    int                                          imageSortOrder;
    int                                          imageSorting;
    int                                          imageSeparationMode;
    int                                          imageSeparationSortOrder;
    int                                          itemLeftClickAction;

    /// Baloo settings
    bool                                         syncToDigikam;
    bool                                         syncToBaloo;

    /// versioning settings

    VersionManagerSettings                       versionSettings;

    /// face detection settings
    double                                       faceDetectionAccuracy;
    bool                                         faceDetectionYoloV3;

    /// misc
    ApplicationSettings::StringComparisonType    stringComparisonType;
    QString                                      applicationStyle;
    QString                                      applicationIcon;
    QFont                                        applicationFont;

    int                                          minimumSimilarityBound;
    int                                          duplicatesSearchLastMinSimilarity;
    int                                          duplicatesSearchLastMaxSimilarity;
    int                                          duplicatesSearchLastAlbumTagRelation;
    int                                          duplicatesSearchLastRestrictions;
    HaarIface::RefImageSelMethod                 duplicatesSearchLastReferenceImageSelectionMethod;

    /// Grouping operation settings
    ApplicationSettings::OperationModes          groupingOperateOnAll;

private:

    ApplicationSettings*                         q;

private:

    // cppcheck-suppress unusedPrivateFunction
    static ApplicationSettings::OperationStrings createConfigGroupingOperateOnAll();
};

} // namespace Digikam

#endif // DIGIKAM_APPLICATION_SETTINGS_PRIVATE_H
