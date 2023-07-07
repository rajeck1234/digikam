/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : A container to hold GPS information about an item.
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_GPS_ITEM_CONTAINER_H
#define DIGIKAM_GPS_ITEM_CONTAINER_H

// Qt includes

#include <QVariant>
#include <QDateTime>
#include <QUrl>

// Local includes

#include "geoifacetypes.h"
#include "digikam_export.h"
#include "gpsdatacontainer.h"
#include "metadatainfo.h"
#include "rginfo.h"

namespace Digikam
{

enum Type
{
    TypeChild    = 1,
    TypeSpacer   = 2,
    TypeNewChild = 4
};

typedef struct TagData
{
    TagData()
        : tagType(TypeChild)
    {
    }

    QString tagName;
    QString tipName;
    Type    tagType;

} TagData;

// --------------------------------------------------------------------------

class SaveProperties
{
public:

    explicit SaveProperties()
      : shouldRemoveCoordinates (false),
        shouldRemoveAltitude    (false),
        shouldWriteCoordinates  (false),
        shouldWriteAltitude     (false),
        altitude                (0.0),
        latitude                (0.0),
        longitude               (0.0)
    {
    }

    bool  shouldRemoveCoordinates;
    bool  shouldRemoveAltitude;
    bool  shouldWriteCoordinates;
    bool  shouldWriteAltitude;
    qreal altitude;
    qreal latitude;
    qreal longitude;
};

// --------------------------------------------------------------------------

class GPSItemModel;
class DMetadata;

class DIGIKAM_EXPORT GPSItemContainer
{
public:

    static const int RoleCoordinates             = Qt::UserRole + 1;

    static const int ColumnThumbnail             = 0;
    static const int ColumnFilename              = 1;
    static const int ColumnDateTime              = 2;
    static const int ColumnLatitude              = 3;
    static const int ColumnLongitude             = 4;
    static const int ColumnAltitude              = 5;
    static const int ColumnAccuracy              = 6;
    static const int ColumnTags                  = 7;
    static const int ColumnStatus                = 8;
    static const int ColumnDOP                   = 9;
    static const int ColumnFixType               = 10;
    static const int ColumnNSatellites           = 11;
    static const int ColumnSpeed                 = 12;

    static const int ColumnGPSItemContainerCount = 13;

public:

    explicit GPSItemContainer(const QUrl& url);
    virtual ~GPSItemContainer();

    /// @name Loading and saving
    //@{
    virtual QString saveChanges();
    virtual bool loadImageData();
    //@}

    bool isDirty()                                                              const;
    QUrl url()                                                                  const;
    QDateTime dateTime()                                                        const;

    /// @name Functions used by the model
    //@{
    static void setHeaderData(GPSItemModel* const model);
    bool lessThan(const GPSItemContainer* const otherItem, const int column)    const;
    //@}

    /// @name GPS related functions
    //@{
    void setCoordinates(const GeoCoordinates& newCoordinates);
    GeoCoordinates coordinates()                                                const;
    GPSDataContainer gpsData()                                                  const;
    void setGPSData(const GPSDataContainer& container);
    void restoreGPSData(const GPSDataContainer& container);
    //@}

    /// @name Tag related functions
    //@{
    /**
     * The tags added in reverse geocoding process are stored in each image, before they end up in external tag model. This function adds them.
     * @param externalTagList A list containing tags.
     */
    void setTagList(const QList<QList<TagData> >& externalTagList);

    /**
     * @return Returns true is the current image has been modified and not saved.
     */
    bool isTagListDirty()                                                       const;

    /**
     * Returns the tag list of the current image.
     */
    QList<QList<TagData> > getTagList()                                         const;

    /**
     * Replaces the current tag list with the one contained in tagList.
     */
    void restoreRGTagList(const QList<QList<TagData> >& tagList);

    /**
     * Writes the current tags to XMP metadata.
     */
    void writeTagsToXmp(const bool writeXmpTags) { m_writeXmpTags = writeXmpTags; }

   /**
     * Writes the current tags to the metadata location fields.
     */
    void writeLocations(const bool writeMetaLoc) { m_writeMetaLoc = writeMetaLoc; }
    //@}

protected:

    void setLocationInfo(const TagData& tagData, IptcCoreLocationInfo& locationInfo);

protected:

    /// these are only to be called by the GPSItemModel
    QVariant data(const int column, const int role)                             const;
    void setModel(GPSItemModel* const model);
    void emitDataChanged();
    DMetadata* getMetadataForFile()                                             const;
    SaveProperties saveProperties()                                             const;

protected:

    GPSItemModel*          m_model;

    QUrl                   m_url;
    QDateTime              m_dateTime;

    bool                   m_dirty;
    GPSDataContainer       m_gpsData;
    GPSDataContainer       m_savedState;

    bool                   m_tagListDirty;
    QList<QList<TagData> > m_tagList;
    QList<QList<TagData> > m_savedTagList;
    bool                   m_writeXmpTags;
    bool                   m_writeMetaLoc;

    bool                   m_saveTags;
    bool                   m_saveGPS;

    friend class GPSItemModel;

private:

    Q_DISABLE_COPY(GPSItemContainer)
};

} // namespace Digikam

#endif // DIGIKAM_GPS_ITEM_CONTAINER_H
