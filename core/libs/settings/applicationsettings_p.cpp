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

#include "applicationsettings_p.h"

namespace Digikam
{

const QString ApplicationSettings::Private::configGroupDefault(QLatin1String("Album Settings"));
const QString ApplicationSettings::Private::configGroupExif(QLatin1String("EXIF Settings"));
const QString ApplicationSettings::Private::configGroupMetadata(QLatin1String("Metadata Settings"));
const QString ApplicationSettings::Private::configGroupBaloo(QLatin1String("Baloo Settings"));
const QString ApplicationSettings::Private::configGroupGeneral(QLatin1String("General Settings"));
const QString ApplicationSettings::Private::configGroupVersioning(QLatin1String("Versioning Settings"));
const QString ApplicationSettings::Private::configGroupFaceDetection(QLatin1String("Face Detection Settings"));
const QString ApplicationSettings::Private::configGroupDuplicatesSearch(QLatin1String("Find Duplicates View"));
const QString ApplicationSettings::Private::configGroupGrouping(QLatin1String("Grouping Behaviour"));
const QString ApplicationSettings::Private::configAlbumCollectionsEntry(QLatin1String("Album Collections"));
const QString ApplicationSettings::Private::configAlbumMonitoringEntry(QLatin1String("Album Monitoring"));
const QString ApplicationSettings::Private::configAlbumSortRoleEntry(QLatin1String("Album Sort Role"));
const QString ApplicationSettings::Private::configImageSortOrderEntry(QLatin1String("Image Sort Order"));
const QString ApplicationSettings::Private::configImageSortingEntry(QLatin1String("Image Sorting"));
const QString ApplicationSettings::Private::configImageSeparationModeEntry(QLatin1String("Image Group Mode"));
const QString ApplicationSettings::Private::configImageSeparationSortOrderEntry(QLatin1String("Image Group Sort Order"));
const QString ApplicationSettings::Private::configItemLeftClickActionEntry(QLatin1String("Item Left Click Action"));
const QString ApplicationSettings::Private::configDefaultIconSizeEntry(QLatin1String("Default Icon Size"));
const QString ApplicationSettings::Private::configDefaultTreeIconSizeEntry(QLatin1String("Default Tree Icon Size"));
const QString ApplicationSettings::Private::configDefaultTreeFaceSizeEntry(QLatin1String("Default Tree Face Size"));
const QString ApplicationSettings::Private::configTreeViewFontEntry(QLatin1String("TreeView Font"));
const QString ApplicationSettings::Private::configThemeEntry(QLatin1String("Theme"));
const QString ApplicationSettings::Private::configUpdateType(QLatin1String("Update Type"));
const QString ApplicationSettings::Private::configUpdateWithDebug(QLatin1String("Update With Debug"));
const QString ApplicationSettings::Private::configSidebarTitleStyleEntry(QLatin1String("Sidebar Title Style"));
const QString ApplicationSettings::Private::configRatingFilterConditionEntry(QLatin1String("Rating Filter Condition"));
const QString ApplicationSettings::Private::configRecursiveAlbumsEntry(QLatin1String("Recursive Albums"));
const QString ApplicationSettings::Private::configRecursiveTagsEntry(QLatin1String("Recursive Tags"));
const QString ApplicationSettings::Private::configAllGroupsOpenEntry(QLatin1String("All Groups Open"));
const QString ApplicationSettings::Private::configIconShowNameEntry(QLatin1String("Icon Show Name"));
const QString ApplicationSettings::Private::configIconShowResolutionEntry(QLatin1String("Icon Show Resolution"));
const QString ApplicationSettings::Private::configIconShowSizeEntry(QLatin1String("Icon Show Size"));
const QString ApplicationSettings::Private::configIconShowDateEntry(QLatin1String("Icon Show Date"));
const QString ApplicationSettings::Private::configIconShowModificationDateEntry(QLatin1String("Icon Show Modification Date"));
const QString ApplicationSettings::Private::configIconShowTitleEntry(QLatin1String("Icon Show Title"));
const QString ApplicationSettings::Private::configIconShowCommentsEntry(QLatin1String("Icon Show Comments"));
const QString ApplicationSettings::Private::configIconShowTagsEntry(QLatin1String("Icon Show Tags"));
const QString ApplicationSettings::Private::configIconShowRatingEntry(QLatin1String("Icon Show Rating"));
const QString ApplicationSettings::Private::configIconShowPickLabelEntry(QLatin1String("Icon Show Pick Label"));
const QString ApplicationSettings::Private::configIconShowColorLabelEntry(QLatin1String("Icon Show Color Label"));
const QString ApplicationSettings::Private::configIconShowImageFormatEntry(QLatin1String("Icon Show Image Format"));
const QString ApplicationSettings::Private::configIconShowCoordinatesEntry(QLatin1String("Icon Show Coordinates"));
const QString ApplicationSettings::Private::configIconShowAspectRatioEntry(QLatin1String("Icon Show Aspect Ratio"));
const QString ApplicationSettings::Private::configIconShowOverlaysEntry(QLatin1String("Icon Show Overlays"));
const QString ApplicationSettings::Private::configIconShowFullscreenEntry(QLatin1String("Icon Show Fullscreen"));
const QString ApplicationSettings::Private::configIconViewFontEntry(QLatin1String("IconView Font"));
const QString ApplicationSettings::Private::configToolTipsFontEntry(QLatin1String("ToolTips Font"));
const QString ApplicationSettings::Private::configShowToolTipsEntry(QLatin1String("Show ToolTips"));
const QString ApplicationSettings::Private::configToolTipsShowFileNameEntry(QLatin1String("ToolTips Show File Name"));
const QString ApplicationSettings::Private::configToolTipsShowFileDateEntry(QLatin1String("ToolTips Show File Date"));
const QString ApplicationSettings::Private::configToolTipsShowFileSizeEntry(QLatin1String("ToolTips Show File Size"));
const QString ApplicationSettings::Private::configToolTipsShowImageTypeEntry(QLatin1String("ToolTips Show Image Type"));
const QString ApplicationSettings::Private::configToolTipsShowImageDimEntry(QLatin1String("ToolTips Show Image Dim"));
const QString ApplicationSettings::Private::configToolTipsShowImageAREntry(QLatin1String("ToolTips Show Image AR"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoMakeEntry(QLatin1String("ToolTips Show Photo Make"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoLensEntry(QLatin1String("ToolTips Show Photo Lens"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoDateEntry(QLatin1String("ToolTips Show Photo Date"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoFocalEntry(QLatin1String("ToolTips Show Photo Focal"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoExpoEntry(QLatin1String("ToolTips Show Photo Expo"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoModeEntry(QLatin1String("ToolTips Show Photo Mode"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoFlashEntry(QLatin1String("ToolTips Show Photo Flash"));
const QString ApplicationSettings::Private::configToolTipsShowPhotoWBEntry(QLatin1String("ToolTips Show Photo WB"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumNameEntry(QLatin1String("ToolTips Show Album Name"));
const QString ApplicationSettings::Private::configToolTipsShowTitlesEntry(QLatin1String("ToolTips Show Titles"));
const QString ApplicationSettings::Private::configToolTipsShowCommentsEntry(QLatin1String("ToolTips Show Comments"));
const QString ApplicationSettings::Private::configToolTipsShowTagsEntry(QLatin1String("ToolTips Show Tags"));
const QString ApplicationSettings::Private::configToolTipsShowLabelRatingEntry(QLatin1String("ToolTips Show Label Rating"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAspectRatioEntry(QLatin1String("ToolTips Show Video Aspect Ratio"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioBitRateEntry(QLatin1String("ToolTips Show Audio Bit Rate"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioChannelTypeEntry(QLatin1String("ToolTips Show Audio Channel Type"));
const QString ApplicationSettings::Private::configToolTipsShowVideoAudioCodecEntry(QLatin1String("ToolTips Show Audio Codec"));
const QString ApplicationSettings::Private::configToolTipsShowVideoDurationEntry(QLatin1String("ToolTips Show Video Duration"));
const QString ApplicationSettings::Private::configToolTipsShowVideoFrameRateEntry(QLatin1String("ToolTips Show Video Frame Rate"));
const QString ApplicationSettings::Private::configToolTipsShowVideoVideoCodecEntry(QLatin1String("ToolTips Show Video Codec"));
const QString ApplicationSettings::Private::configShowAlbumToolTipsEntry(QLatin1String("Show Album ToolTips"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumTitleEntry(QLatin1String("ToolTips Show Album Title"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumDateEntry(QLatin1String("ToolTips Show Album Date"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumCollectionEntry(QLatin1String("ToolTips Show Album Collection"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumCategoryEntry(QLatin1String("ToolTips Show Album Category"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumCaptionEntry(QLatin1String("ToolTips Show Album Caption"));
const QString ApplicationSettings::Private::configToolTipsShowAlbumPreviewEntry(QLatin1String("ToolTips Show Album Preview"));
const QString ApplicationSettings::Private::configPreviewLoadFullItemSizeEntry(QLatin1String("Preview Load Full Image Size"));
const QString ApplicationSettings::Private::configPreviewRawUseLoadingDataEntry(QLatin1String("Preview Raw Use Loading Data"));
const QString ApplicationSettings::Private::configPreviewConvertToEightBitEntry(QLatin1String("Preview Convert To Eight Bit"));
const QString ApplicationSettings::Private::configPreviewScaleFitToWindowEntry(QLatin1String("Preview Scale Fit To Window"));
const QString ApplicationSettings::Private::configPreviewShowIconsEntry(QLatin1String("Preview Show Icons"));
const QString ApplicationSettings::Private::configShowThumbbarEntry(QLatin1String("Show Thumbbar"));
const QString ApplicationSettings::Private::configShowFolderTreeViewItemsCountEntry(QLatin1String("Show Folder Tree View Items Count"));
const QString ApplicationSettings::Private::configShowSplashEntry(QLatin1String("Show Splash"));
const QString ApplicationSettings::Private::configUseTrashEntry(QLatin1String("Use Trash"));
const QString ApplicationSettings::Private::configShowTrashDeleteDialogEntry(QLatin1String("Show Trash Delete Dialog"));
const QString ApplicationSettings::Private::configShowPermanentDeleteDialogEntry(QLatin1String("Show Permanent Delete Dialog"));
const QString ApplicationSettings::Private::configApplySidebarChangesDirectlyEntry(QLatin1String("Apply Sidebar Changes Directly"));
const QString ApplicationSettings::Private::configUseNativeFileDialogEntry(QLatin1String("Use Native File Dialog"));
const QString ApplicationSettings::Private::configDrawFramesToGroupedEntry(QLatin1String("Draw Frames To Grouped Items"));
const QString ApplicationSettings::Private::configExpandNewCurrentItemEntry(QLatin1String("Expand New Current Item On Click"));
const QString ApplicationSettings::Private::configScrollItemToCenterEntry(QLatin1String("Scroll Current Item To Center"));
const QString ApplicationSettings::Private::configShowOnlyPersonTagsInPeopleSidebarEntry(QLatin1String("Show Only Face Tags For Assigning Name"));
const QString ApplicationSettings::Private::configDetectFacesInNewImagesEntry(QLatin1String("Detect faces in newly added images"));
const QString ApplicationSettings::Private::configSyncBalootoDigikamEntry(QLatin1String("Sync Baloo to Digikam"));
const QString ApplicationSettings::Private::configSyncDigikamtoBalooEntry(QLatin1String("Sync Digikam to Baloo"));
const QString ApplicationSettings::Private::configStringComparisonTypeEntry(QLatin1String("String Comparison Type"));
const QString ApplicationSettings::Private::configFaceDetectionAccuracyEntry(QLatin1String("Detection Accuracy"));
const QString ApplicationSettings::Private::configFaceDetectionYoloV3Entry(QLatin1String("Use Yolo V3"));
const QString ApplicationSettings::Private::configApplicationStyleEntry(QLatin1String("Application Style"));
const QString ApplicationSettings::Private::configIconThemeEntry(QLatin1String("Icon Theme"));
const QString ApplicationSettings::Private::configApplicationFontEntry(QLatin1String("Application Font"));
const QString ApplicationSettings::Private::configScanAtStartEntry(QLatin1String("Scan At Start"));
const QString ApplicationSettings::Private::configCleanAtStartEntry(QLatin1String("Clean core DB At Start"));
const QString ApplicationSettings::Private::configMinimumSimilarityBound(QLatin1String("Lower bound for minimum similarity"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastMinSimilarity(QLatin1String("Last minimum similarity"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastMaxSimilarity(QLatin1String("Last maximum similarity"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastAlbumTagRelation(QLatin1String("Last search album tag relation"));
const QString ApplicationSettings::Private::configDuplicatesSearchLastRestrictions(QLatin1String("Last search results restriction"));
const QString ApplicationSettings::Private::configDuplicatesSearchReferenceSelectionMethod(QLatin1String("Last reference image method"));
const ApplicationSettings::OperationStrings ApplicationSettings::Private::configGroupingOperateOnAll =
        ApplicationSettings::Private::createConfigGroupingOperateOnAll();

ApplicationSettings::OperationStrings ApplicationSettings::Private::createConfigGroupingOperateOnAll()
{
    ApplicationSettings::OperationStrings out;
    out.insert(ApplicationSettings::Metadata,     QLatin1String("Do metadata operations on all"));
    out.insert(ApplicationSettings::ImportExport, QLatin1String("Do Import Export operations on all"));
    out.insert(ApplicationSettings::BQM,          QLatin1String("Do BQM operations on all"));
    out.insert(ApplicationSettings::LightTable,   QLatin1String("Do light table operations on all"));
    out.insert(ApplicationSettings::Slideshow,    QLatin1String("Do slideshow operations on all"));
    out.insert(ApplicationSettings::Rename,       QLatin1String("Rename all"));
    out.insert(ApplicationSettings::Tools,        QLatin1String("Operate tools on all"));

    return out;
}

ApplicationSettings::Private::Private(ApplicationSettings* const qq)
    : showSplash                                        (false),
      useTrash                                          (false),
      showTrashDeleteDialog                             (false),
      showPermanentDeleteDialog                         (false),
      sidebarApplyDirectly                              (false),
      useNativeFileDialog                               (false),
      drawFramesToGrouped                               (true),
      expandNewCurrentItem                              (false),
      scrollItemToCenter                                (false),
      showOnlyPersonTagsInPeopleSidebar                 (false),
      detectFacesInNewImages                            (false),
      iconShowName                                      (false),
      iconShowSize                                      (false),
      iconShowDate                                      (false),
      iconShowModDate                                   (false),
      iconShowTitle                                     (false),
      iconShowComments                                  (false),
      iconShowResolution                                (false),
      iconShowTags                                      (false),
      iconShowOverlays                                  (false),
      iconShowFullscreen                                (false),
      iconShowRating                                    (false),
      iconShowPickLabel                                 (false),
      iconShowColorLabel                                (false),
      iconShowImageFormat                               (false),
      iconShowCoordinates                               (false),
      iconShowAspectRatio                               (false),
      showToolTips                                      (false),
      tooltipShowFileName                               (false),
      tooltipShowFileDate                               (false),
      tooltipShowFileSize                               (false),
      tooltipShowImageType                              (false),
      tooltipShowImageDim                               (false),
      tooltipShowImageAR                                (false),
      tooltipShowPhotoMake                              (false),
      tooltipShowPhotoLens                              (false),
      tooltipShowPhotoDate                              (false),
      tooltipShowPhotoFocal                             (false),
      tooltipShowPhotoExpo                              (false),
      tooltipShowPhotoMode                              (false),
      tooltipShowPhotoFlash                             (false),
      tooltipShowPhotoWb                                (false),
      tooltipShowAlbumName                              (false),
      tooltipShowTitles                                 (false),
      tooltipShowComments                               (false),
      tooltipShowTags                                   (false),
      tooltipShowLabelRating                            (false),
      tooltipShowVideoAspectRatio                       (false),
      tooltipShowVideoAudioBitRate                      (false),
      tooltipShowVideoAudioChannelType                  (false),
      tooltipShowVideoAudioCodec                        (false),
      tooltipShowVideoDuration                          (false),
      tooltipShowVideoFrameRate                         (false),
      tooltipShowVideoVideoCodec                        (false),
      showAlbumToolTips                                 (false),
      tooltipShowAlbumTitle                             (false),
      tooltipShowAlbumDate                              (false),
      tooltipShowAlbumCollection                        (false),
      tooltipShowAlbumCategory                          (false),
      tooltipShowAlbumCaption                           (false),
      tooltipShowAlbumPreview                           (false),
      scaleFitToWindow                                  (false),
      previewShowIcons                                  (true),
      showThumbbar                                      (false),
      showFolderTreeViewItemsCount                      (false),
      treeThumbnailSize                                 (0),
      treeThumbFaceSize                                 (0),
      thumbnailSize                                     (0),
      ratingFilterCond                                  (0),
      recursiveAlbums                                   (false),
      recursiveTags                                     (false),
      allGroupsOpen                                     (false),
      scanAtStart                                       (true),
      cleanAtStart                                      (true),
      databaseDirSetAtCmd                               (false),
      albumMonitoring                                   (false),
      updateType                                        (0),
      updateWithDebug                                   (false),
      sidebarTitleStyle                                 (DMultiTabBar::AllIconsText),
      albumSortRole                                     (ApplicationSettings::ByFolder),
      albumSortChanged                                  (false),
      imageSortOrder                                    (0),
      imageSorting                                      (0),
      imageSeparationMode                               (0),
      imageSeparationSortOrder                          (0),
      itemLeftClickAction                               (ApplicationSettings::ShowPreview),
      syncToDigikam                                     (false),
      syncToBaloo                                       (false),
      faceDetectionAccuracy                             (0.7),
      faceDetectionYoloV3                               (false),
      stringComparisonType                              (ApplicationSettings::Natural),
      minimumSimilarityBound                            (40),
      duplicatesSearchLastMinSimilarity                 (90),
      duplicatesSearchLastMaxSimilarity                 (100),
      duplicatesSearchLastAlbumTagRelation              (0),
      duplicatesSearchLastRestrictions                  (0),
      duplicatesSearchLastReferenceImageSelectionMethod (HaarIface::RefImageSelMethod::OlderOrLarger),
      groupingOperateOnAll                              (ApplicationSettings::OperationModes()),
      q                                                 (qq)
{
    for (int i = 0 ; i != ApplicationSettings::Unspecified ; ++i)
    {
        groupingOperateOnAll.insert((ApplicationSettings::OperationType)i,
                ApplicationSettings::Ask);
    }
}

ApplicationSettings::Private::~Private()
{
}

void ApplicationSettings::Private::init()
{
    albumCategoryNames.clear();
    albumCategoryNames.append(i18nc("@item: album categories", "Category"));
    albumCategoryNames.append(i18nc("@item: album categories", "Travel"));
    albumCategoryNames.append(i18nc("@item: album categories", "Holidays"));
    albumCategoryNames.append(i18nc("@item: album categories", "Friends"));
    albumCategoryNames.append(i18nc("@item: album categories", "Nature"));
    albumCategoryNames.append(i18nc("@item: album categories", "Party"));
    albumCategoryNames.append(i18nc("@item: album categories", "Todo"));
    albumCategoryNames.append(i18nc("@item: album categories", "Miscellaneous"));
    albumCategoryNames.sort();

    albumSortRole                                     = ApplicationSettings::ByFolder;
    imageSortOrder                                    = ItemSortSettings::SortByFileName;
    imageSorting                                      = ItemSortSettings::AscendingOrder;
    imageSeparationMode                               = ItemSortSettings::CategoryByAlbum;
    imageSeparationSortOrder                          = ItemSortSettings::AscendingOrder;

    itemLeftClickAction                               = ApplicationSettings::ShowPreview;

    thumbnailSize                                     = ThumbnailSize::Medium;
    treeThumbnailSize                                 = 22;
    treeThumbFaceSize                                 = 48;
    treeviewFont                                      = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    updateType                                        = 0;
    updateWithDebug                                   = false;
    sidebarTitleStyle                                 = DMultiTabBar::AllIconsText;

    ratingFilterCond                                  = ItemFilterSettings::GreaterEqualCondition;

    showSplash                                        = true;
    useTrash                                          = true;
    showTrashDeleteDialog                             = true;
    showPermanentDeleteDialog                         = true;
    sidebarApplyDirectly                              = false;
    useNativeFileDialog                               = false;
    drawFramesToGrouped                               = true;
    expandNewCurrentItem                              = true;
    scrollItemToCenter                                = false;
    showOnlyPersonTagsInPeopleSidebar                 = false;
    detectFacesInNewImages                            = false;

    iconShowName                                      = true;
    iconShowSize                                      = false;
    iconShowDate                                      = true;
    iconShowModDate                                   = false;
    iconShowTitle                                     = true;
    iconShowComments                                  = true;
    iconShowResolution                                = false;
    iconShowAspectRatio                               = false;
    iconShowTags                                      = true;
    iconShowOverlays                                  = true;
    iconShowFullscreen                                = true;
    iconShowRating                                    = true;
    iconShowPickLabel                                 = false;
    iconShowColorLabel                                = true;
    iconShowImageFormat                               = true;
    iconShowCoordinates                               = true;
    iconviewFont                                      = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    toolTipsFont                                      = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
    showToolTips                                      = false;
    tooltipShowFileName                               = true;
    tooltipShowFileDate                               = false;
    tooltipShowFileSize                               = false;
    tooltipShowImageType                              = false;
    tooltipShowImageDim                               = true;
    tooltipShowImageAR                                = true;
    tooltipShowPhotoMake                              = true;
    tooltipShowPhotoLens                              = true;
    tooltipShowPhotoDate                              = true;
    tooltipShowPhotoFocal                             = true;
    tooltipShowPhotoExpo                              = true;
    tooltipShowPhotoMode                              = true;
    tooltipShowPhotoFlash                             = false;
    tooltipShowPhotoWb                                = false;
    tooltipShowAlbumName                              = false;
    tooltipShowTitles                                 = false;
    tooltipShowComments                               = true;
    tooltipShowTags                                   = true;
    tooltipShowLabelRating                            = true;

    tooltipShowVideoAspectRatio                       = true;
    tooltipShowVideoAudioBitRate                      = true;
    tooltipShowVideoAudioChannelType                  = true;
    tooltipShowVideoAudioCodec                        = true;
    tooltipShowVideoDuration                          = true;
    tooltipShowVideoFrameRate                         = true;
    tooltipShowVideoVideoCodec                        = true;

    showAlbumToolTips                                 = false;
    tooltipShowAlbumTitle                             = true;
    tooltipShowAlbumDate                              = true;
    tooltipShowAlbumCollection                        = true;
    tooltipShowAlbumCategory                          = true;
    tooltipShowAlbumCaption                           = true;
    tooltipShowAlbumPreview                           = false;

    scaleFitToWindow                                  = false;
    previewShowIcons                                  = true;
    showThumbbar                                      = true;

    recursiveAlbums                                   = false;
    recursiveTags                                     = true;
    allGroupsOpen                                     = false;

    showFolderTreeViewItemsCount                      = false;

    syncToDigikam                                     = false;
    syncToBaloo                                       = false;
    albumSortChanged                                  = false;

    faceDetectionAccuracy                             = 0.7;
    faceDetectionYoloV3                               = false;

    minimumSimilarityBound                            = 40;
    duplicatesSearchLastMinSimilarity                 = 90;
    duplicatesSearchLastMaxSimilarity                 = 100;
    duplicatesSearchLastAlbumTagRelation              = 0;
    duplicatesSearchLastRestrictions                  = 0;
    duplicatesSearchLastReferenceImageSelectionMethod = HaarIface::RefImageSelMethod::OlderOrLarger;

    scanAtStart                                       = true;
    cleanAtStart                                      = true;
    databaseDirSetAtCmd                               = false;
    albumMonitoring                                   = false;
    stringComparisonType                              = ApplicationSettings::Natural;

    applicationStyle                                  = qApp->style()->objectName();
    applicationIcon                                   = QString();
    applicationFont                                   = QFontDatabase::systemFont(QFontDatabase::GeneralFont);

    for (int i = 0 ; i != ApplicationSettings::Unspecified ; ++i)
    {
        groupingOperateOnAll.insert((ApplicationSettings::OperationType)i,
                                    ApplicationSettings::Ask);
    }

    q->connect(q, SIGNAL(balooSettingsChanged()),
               q, SLOT(applyBalooSettings()));
}

} // namespace Digikam
