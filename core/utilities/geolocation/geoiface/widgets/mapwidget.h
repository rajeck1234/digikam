/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-01
 * Description : world map widget library
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2009-2011 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText:      2014 by Justus Schwartz <justus at gmx dot li>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MAP_WIDGET_H
#define DIGIKAM_MAP_WIDGET_H

// Qt includes

#include <QWidget>
#include <QStringList>

// local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "geogroupstate.h"
#include "digikam_export.h"

class QAction;
class QDragEnterEvent;
class QDropEvent;
class QMenu;

class KConfigGroup;

namespace Digikam
{

class GeoIfaceSharedData;
class GeoModelHelper;
class GeoDragDropHandler;
class AbstractMarkerTiler;
class TrackManager;

class DIGIKAM_EXPORT MapWidget : public QWidget
{
  Q_OBJECT

public:

    explicit MapWidget(QWidget* const parent = nullptr);
    ~MapWidget() override;

    void saveSettingsToGroup(KConfigGroup* const group);
    void readSettingsFromGroup(const KConfigGroup* const group);

    /// @name Data
    //@{
    void addUngroupedModel(GeoModelHelper* const modelHelper);
    void removeUngroupedModel(GeoModelHelper* const modelHelper);
    void setGroupedModel(AbstractMarkerTiler* const markerModel);
    void setDragDropHandler(GeoDragDropHandler* const dragDropHandler);
    void setTrackManager(TrackManager* const trackManager);
    //@}

    /// @name UI setup
    //@{
    QAction* getControlAction(const QString& actionName);
    QWidget* getControlWidget();
    void addWidgetToControlWidget(QWidget* const newWidget);
    void setSortOptionsMenu(QMenu* const sortMenu);
    void setMouseMode(const GeoMouseModes mouseMode);
    void setAvailableMouseModes(const GeoMouseModes mouseModes);
    void setVisibleMouseModes(const GeoMouseModes mouseModes);
    void setAllowModifications(const bool state);
    void setActive(const bool state);
    bool getActiveState();
    bool getStickyModeState() const;
    void setStickyModeState(const bool state);
    void setVisibleExtraActions(const GeoExtraActions actions);
    void setEnabledExtraActions(const GeoExtraActions actions);
    //@}

    /// @name Map related functions
    //@{
    QStringList availableBackends() const;
    bool setBackend(const QString& backendName);

    GeoCoordinates getCenter() const;
    void setCenter(const GeoCoordinates& coordinate);

    void setZoom(const QString& newZoom);
    QString getZoom();

    void adjustBoundariesToGroupedMarkers(const bool useSaneZoomLevel = true);
    void refreshMap();
    //@}

    /// @name Appearance
    //@{
    void setSortKey(const int sortKey);
    void setThumnailSize(const int newThumbnailSize);
    void setThumbnailGroupingRadius(const int newGroupingRadius);
    void setMarkerGroupingRadius(const int newGroupingRadius);
    int  getThumbnailSize() const;
    int  getUndecoratedThumbnailSize() const;
    void setShowThumbnails(const bool state);
    //@}

    /// @name Region selection
    //@{
    void setRegionSelection(const GeoCoordinates::Pair& region);
    GeoCoordinates::Pair getRegionSelection();
    void clearRegionSelection();
    //@}

    /**
     * @name Internal
     * Functions that are only used internally and should be hidden from the public interface
     */
    //@{
    void updateMarkers();
    void updateClusters();
    void markClustersAsDirty();

    void getColorInfos(const int clusterIndex, QColor* fillColor, QColor* strokeColor,
                       Qt::PenStyle* strokeStyle, QString* labelText, QColor* labelColor,
                       const GeoGroupState* const overrideSelection = nullptr,
                       const int* const overrideCount = nullptr) const;

    void getColorInfos(const GeoGroupState groupState,
                       const int nMarkers,
                       QColor* fillColor, QColor* strokeColor,
                       Qt::PenStyle* strokeStyle, QString* labelText, QColor* labelColor) const;

    QString convertZoomToBackendZoom(const QString& someZoom, const QString& targetBackend) const;
    QPixmap getDecoratedPixmapForCluster(const int clusterId, const GeoGroupState* const selectedStateOverride,
                                         const int* const countOverride, QPoint* const centerPoint);
    QVariant getClusterRepresentativeMarker(const int clusterIndex, const int sortKey);
    //@}

public Q_SLOTS:

    /// @name Appearance
    //@{
    void slotZoomIn();
    void slotZoomOut();
    void slotDecreaseThumbnailSize();
    void slotIncreaseThumbnailSize();
    //@}

    /// @name Internal?
    //@{
    void slotUpdateActionsEnabled();
    void slotClustersNeedUpdating();
    void stopThumbnailTimer();
    void slotStickyModeChanged();
    //@}

Q_SIGNALS:

    void signalUngroupedModelChanged(const int index);
    void signalRegionSelectionChanged();
    void signalRemoveCurrentFilter();
    void signalStickyModeChanged();
    void signalMouseModeChanged(const Digikam::GeoMouseModes& currentMouseMode);
    void signalLoadTracksFromAlbums();

public:

    /**
     * Return a string version of LibMarbleWidget release in format "major.minor.patch"
     */
    static QString MarbleWidgetVersion();

protected:

    bool currentBackendReady() const;
    void applyCacheToBackend();
    void saveBackendToCache();
    void rebuildConfigurationMenu();
    void dropEvent(QDropEvent* event)           override;
    void dragMoveEvent(QDragMoveEvent* event)   override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dragLeaveEvent(QDragLeaveEvent* event) override;
    void createActions();
    void createActionsForBackendSelection();
    void setShowPlaceholderWidget(const bool state);
    void setMapWidgetInFrame(QWidget* const widgetForFrame);
    void removeMapWidgetFromFrame();

protected Q_SLOTS:

    void slotBackendReadyChanged(const QString& backendName);
    void slotChangeBackend(QAction* action);
    void slotBackendZoomChanged(const QString& newZoom);
    void slotClustersMoved(const QIntList& clusterIndices, const QPair<int, QModelIndex>& snapTarget);
    void slotClustersClicked(const QIntList& clusterIndices);
    void slotShowThumbnailsChanged();
    void slotRequestLazyReclustering();
    void slotLazyReclusteringRequestCallBack();
    void slotItemDisplaySettingsChanged();
    void slotUngroupedModelChanged();
    void slotNewSelectionFromMap(const Digikam::GeoCoordinates::Pair& sel);

    /// @name Mouse modes
    //@{
    void slotMouseModeChanged(QAction* triggeredAction);
    void slotRemoveCurrentRegionSelection();
    //@}

private:

    const QExplicitlySharedDataPointer<GeoIfaceSharedData> s;

    class Private;
    Private* const                                         d;

    Q_DISABLE_COPY(MapWidget)
};

} // namespace Digikam

#endif // DIGIKAM_MAP_WIDGET_H
