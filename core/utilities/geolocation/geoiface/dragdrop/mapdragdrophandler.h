/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-22
 * Description : Drag-and-drop handler for geolocation interface integration.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MAP_DRAG_DROP_HANDLER_H
#define DIGIKAM_MAP_DRAG_DROP_HANDLER_H

// Qt includes

#include <QAbstractItemModel>
#include <QMimeData>

// Local includes

#include "geodragdrophandler.h"
#include "digikam_export.h"

namespace Digikam
{

class GPSGeoIfaceModelHelper;

class DIGIKAM_EXPORT MapDragData : public QMimeData
{
    Q_OBJECT

public:

    explicit MapDragData()
      : QMimeData     (),
        draggedIndices()
    {
    }

    QList<QPersistentModelIndex> draggedIndices;

private:

    // Disable
    explicit MapDragData(QObject*) = delete;
};

//  -----------------------------------------------------------------------------------

class DIGIKAM_EXPORT MapDragDropHandler : public GeoDragDropHandler
{
    Q_OBJECT

public:

    explicit MapDragDropHandler(QAbstractItemModel* const /*pModel*/,
                                GPSGeoIfaceModelHelper* const parent);
    ~MapDragDropHandler()                                                       override;

    Qt::DropAction accepts(const QDropEvent* e)                                 override;
    bool dropEvent(const QDropEvent* e, const GeoCoordinates& dropCoordinates)  override;
    QMimeData* createMimeData(const QList<QPersistentModelIndex>& modelIndices) override;

private:

    GPSGeoIfaceModelHelper* const gpsGeoIfaceModelHelper;
};

} // namespace Digikam

#endif // DIGIKAM_MAP_DRAG_DROP_HANDLER_H
