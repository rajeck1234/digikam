/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-26-07
 * Description : Main view for import tool
 *
 * SPDX-FileCopyrightText: 2012      by Islam Wazery <wazery at ubuntu dot com>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMPORTVIEW_H
#define DIGIKAM_IMPORTVIEW_H

// Local includes

#include "dlayoutbox.h"
#include "camiteminfo.h"
#include "sidebarwidget.h"
#include "importui.h"
#include "importstackedview.h"

namespace Digikam
{

class ImportUI;

class ImportView : public DHBox
{
    Q_OBJECT

public:

    ImportView(Digikam::ImportUI* const ui, QWidget* const parent);
    ~ImportView() override;

    void applySettings();
    void refreshView();
    void clearHistory();
    void getForwardHistory(QStringList& titles);
    void getBackwardHistory(QStringList& titles);
    void showSideBars();
    void hideSideBars();
    void setThumbSize(int size);
    void toggleShowBar(bool b);
    void toggleFullScreen(bool set);
    void updateIconView();

    void scrollTo(const QString& folder, const QString& file);

    CamItemInfo  camItemInfo(const QString& folder, const QString& file)     const;
    CamItemInfo& camItemInfoRef(const QString& folder, const QString& file)  const;
    bool         hasImage(const CamItemInfo& info)                           const;

    QList<QUrl>         allUrls()                                            const;
    QList<QUrl>         selectedUrls()                                       const;
    QList<CamItemInfo> selectedCamItemInfos()                                const;
    QList<CamItemInfo> allItems()                                            const;
    void               setSelectedCamItemInfos(const CamItemInfoList& infos) const;
    int                downloadedCamItemInfos()                              const;
    bool               hasCurrentItem()                                      const;
    bool               isSelected(const QUrl& url)                           const;

    double                             zoomMin()                             const;
    double                             zoomMax()                             const;
    ThumbnailSize                      thumbnailSize()                       const;
    ImportStackedView::StackedViewMode viewMode()                            const;
    ImportFilterModel*                 importFilterModel()                   const;

Q_SIGNALS:

    void signalImageSelected(const CamItemInfoList& selectedImage, const CamItemInfoList& allImages);
    void signalNewSelection(bool hasSelection);
    void signalNoCurrentItem();
    void signalSelectionChanged(int numberOfSelectedItems);
    void signalThumbSizeChanged(int);
    void signalZoomChanged(double);
    void signalSwitchedToPreview();
    void signalSwitchedToIconView();
    void signalSwitchedToMapView();

public Q_SLOTS:

    void setZoomFactor(double zoom);

    /// View Action slots
    void slotZoomIn();
    void slotZoomOut();
    void slotZoomTo100Percents();
    void slotFitToWindow();

    void slotImagePreview();
    void slotMapWidgetView();
    void slotIconView();

    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();

    void slotImageRename();

    void slotSortImagesBy(int sortBy);
    void slotSortImagesOrder(int order);
    void slotSeparateImages(int mode);

private Q_SLOTS:

    void slotImageSelected();
    void slotTogglePreviewMode(const CamItemInfo& info, bool downloadPreview);
    void slotDispatchImageSelected();

    void slotFirstItem();
    void slotPrevItem();
    void slotNextItem();
    void slotLastItem();
    void slotSelectItemByUrl(const QUrl&);

    void slotViewModeChanged();
    void slotEscapePreview();

    void slotThumbSizeEffect();
    void slotZoomFactorChanged(double);

    void slotImageChangeFailed(const QString& message, const QStringList& fileNames);

private:

    void toggleZoomActions();
    void setupConnections();
    void loadViewState();
    void saveViewState();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMPORTVIEW_H
