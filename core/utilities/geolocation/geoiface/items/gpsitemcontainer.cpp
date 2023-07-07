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

#include "gpsitemcontainer.h"

// Qt includes

#include <QBrush>
#include <QFileInfo>
#include <QScopedPointer>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "gpsitemmodel.h"
#include "dmetadata.h"
#include "metaenginesettings.h"

namespace Digikam
{

bool setExifXmpTagDataVariant(DMetadata* const meta, const char* const exifTagName,
                              const char* const xmpTagName, const QVariant& value)
{
    bool success = meta->setExifTagVariant(exifTagName, value);

    if (success)
    {
        /**
         * @todo Here we save all data types as XMP Strings. Is that okay or do we have to store them as some other type?
         */
        switch (value.type())
        {
            case QVariant::Int:
            case QVariant::UInt:
            case QVariant::Bool:
            case QVariant::LongLong:
            case QVariant::ULongLong:
                success = meta->setXmpTagString(xmpTagName, QString::number(value.toInt()));
                break;

            case QVariant::Double:
            {
                long num, den;
                meta->convertToRationalSmallDenominator(value.toDouble(), &num, &den);
                success = meta->setXmpTagString(xmpTagName, QString::fromLatin1("%1/%2").arg(num).arg(den));
                break;
            }

            case QVariant::List:
            {
                long num             = 0;
                long den             = 1;
                QList<QVariant> list = value.toList();

                if (list.size() >= 1)
                {
                    num = list[0].toInt();
                }

                if (list.size() >= 2)
                {
                    den = list[1].toInt();
                }

                success = meta->setXmpTagString(xmpTagName, QString::fromLatin1("%1/%2").arg(num).arg(den));
                break;
            }

            case QVariant::Date:
            case QVariant::DateTime:
            {
                QDateTime dateTime = value.toDateTime();

                if (!dateTime.isValid())
                {
                    success = false;
                    break;
                }

                success = meta->setXmpTagString(xmpTagName, dateTime.toString(QLatin1String("yyyy:MM:dd hh:mm:ss")));
                break;
            }

            case QVariant::String:
            case QVariant::Char:
                success = meta->setXmpTagString(xmpTagName, value.toString());
                break;

            case QVariant::ByteArray:

                /// @todo I don't know a straightforward way to convert a byte array to XMP

                success = false;
                break;

            default:
                success = false;
                break;
        }
    }

    return success;
}

GPSItemContainer::GPSItemContainer(const QUrl& url)
    : m_model       (nullptr),
      m_url         (url),
      m_dateTime    (),
      m_dirty       (false),
      m_gpsData     (),
      m_savedState  (),
      m_tagListDirty(false),
      m_tagList     (),
      m_savedTagList(),
      m_writeXmpTags(true),
      m_writeMetaLoc(true),
      m_saveTags    (true),
      m_saveGPS     (true)
{
}

GPSItemContainer::~GPSItemContainer()
{
}

DMetadata* GPSItemContainer::getMetadataForFile() const
{
    QScopedPointer<DMetadata> meta(new DMetadata);

    if (!meta->load(m_url.toLocalFile()))
    {
        // It is possible that no sidecar file has yet been created.
        // If writing to sidecar file is activated, we ignore the loading error of the metadata.

        if (MetaEngineSettings::instance()->settings().metadataWritingMode == DMetadata::WRITE_TO_FILE_ONLY)
        {
            return nullptr;
        }
    }

    return meta.take();
}

int getWarningLevelFromGPSDataContainer(const GPSDataContainer& data)
{
    if      (data.hasDop())
    {
        const int dopValue = data.getDop();

        if (dopValue < 2)
        {
            return 1;
        }

        if (dopValue < 4)
        {
            return 2;
        }

        if (dopValue < 10)
        {
            return 3;
        }

        return 4;
    }
    else if (data.hasFixType())
    {
        if (data.getFixType() < 3)
        {
            return 4;
        }
    }
    else if (data.hasNSatellites())
    {
        if (data.getNSatellites() < 4)
        {
            return 4;
        }
    }

    // no warning level

    return -1;
}

bool GPSItemContainer::loadImageData()
{
    QScopedPointer<DMetadata> meta(getMetadataForFile());

    if (meta && !m_dateTime.isValid())
    {
        m_dateTime = meta->getItemDateTime();
    }

    if (!m_dateTime.isValid())
    {
        // Get date from filesystem.

        QFileInfo info(m_url.toLocalFile());

        QDateTime ctime = info.birthTime();
        QDateTime mtime = info.lastModified();

        if (ctime.isNull() || mtime.isNull())
        {
            m_dateTime = qMax(ctime, mtime);
        }
        else
        {
            m_dateTime = qMin(ctime, mtime);
        }
    }

    if (!meta)
    {
        return false;
    }

    // The way we read the coordinates here is problematic
    // if the coordinates were in the file initially, but
    // the user deleted them in the database. Then we still load
    // them from the file. On the other hand, we can not clear
    // the coordinates, because then we would loose them if
    // they are only stored in the database.
/*
    m_gpsData.clear();
*/

    if (!m_gpsData.hasCoordinates())
    {
        // could not load the coordinates from the interface,
        // read them directly from the file

        double lat, lng;
        bool haveCoordinates = meta->getGPSLatitudeNumber(&lat) && meta->getGPSLongitudeNumber(&lng);

        if (haveCoordinates)
        {
            GeoCoordinates coordinates(lat, lng);
            double alt;

            if (meta->getGPSAltitude(&alt))
            {
                coordinates.setAlt(alt);
            }

            m_gpsData.setCoordinates(coordinates);
        }
    }

    /**
     * @todo It seems that exiv2 provides EXIF entries if XMP sidecar entries exist,
     * therefore no need to read XMP as well?
     */

    // read the remaining GPS information from the file:

    const QByteArray speedRef  = meta->getExifTagData("Exif.GPSInfo.GPSSpeedRef");
    bool success               = !speedRef.isEmpty();
    long num, den;
    success                   &= meta->getExifTagRational("Exif.GPSInfo.GPSSpeed", num, den);

    if (success)
    {
        // be relaxed about 0/0

        if ((num == 0.0) && (den == 0.0))
        {
            den = 1.0;
        }

        const qreal speedInRef = qreal(num)/qreal(den);
        qreal FactorToMetersPerSecond;

        if      (speedRef.startsWith('K'))
        {
            // km/h = 1000 * 3600

            FactorToMetersPerSecond = 1.0 / 3.6;
        }
        else if (speedRef.startsWith('M'))
        {
            // TODO: someone please check that this is the 'right' mile
            // miles/hour = 1609.344 meters / hour = 1609.344 meters / 3600 seconds

            FactorToMetersPerSecond = 1.0 / (1609.344 / 3600.0);
        }
        else if (speedRef.startsWith('N'))
        {
            // speed is in knots.
            // knot = one nautical mile / hour = 1852 meters / hour = 1852 meters / 3600 seconds

            FactorToMetersPerSecond = 1.0 / (1852.0 / 3600.0);
        }
        else
        {
            success = false;
        }

        if (success)
        {
            const qreal speedInMetersPerSecond = speedInRef * FactorToMetersPerSecond;
            m_gpsData.setSpeed(speedInMetersPerSecond);
        }
    }

    // number of satellites

    const QString gpsSatellitesString = meta->getExifTagString("Exif.GPSInfo.GPSSatellites");
    bool satellitesOkay               = !gpsSatellitesString.isEmpty();

    if (satellitesOkay)
    {
        /**
         * @todo Here we only accept a single integer denoting the number of satellites used
         *       but not detailed information about all satellites.
         */

        const int nSatellites = gpsSatellitesString.toInt(&satellitesOkay);

        if (satellitesOkay)
        {
            m_gpsData.setNSatellites(nSatellites);
        }
    }

    // fix type / measure mode

    const QByteArray gpsMeasureModeByteArray = meta->getExifTagData("Exif.GPSInfo.GPSMeasureMode");
    bool measureModeOkay                     = !gpsMeasureModeByteArray.isEmpty();

    if (measureModeOkay)
    {
        const int measureMode = gpsMeasureModeByteArray.toInt(&measureModeOkay);

        if (measureModeOkay)
        {
            if ((measureMode == 2) || (measureMode == 3))
            {
                m_gpsData.setFixType(measureMode);
            }
        }
    }

    // read the DOP value:

    success = meta->getExifTagRational("Exif.GPSInfo.GPSDOP", num, den);

    if (success)
    {
        // be relaxed about 0/0

        if ((num == 0.0) && (den == 0.0))
        {
            den = 1.0;
        }

        const qreal dop = qreal(num)/qreal(den);

        m_gpsData.setDop(dop);
    }

    // mark us as not-dirty, because the data was just loaded:

    m_dirty      = false;
    m_savedState = m_gpsData;

    emitDataChanged();

    return true;
}

QVariant GPSItemContainer::data(const int column, const int role) const
{
    if      ((column == ColumnFilename) && (role == Qt::DisplayRole))
    {
        return m_url.fileName();
    }
    else if ((column == ColumnDateTime) && (role == Qt::DisplayRole))
    {
        if (m_dateTime.isValid())
        {
            return QLocale().toString(m_dateTime, QLocale::ShortFormat);
        }

        return i18n("Not available");
    }
    else if (role == RoleCoordinates)
    {
        return QVariant::fromValue(m_gpsData.getCoordinates());
    }
    else if ((column == ColumnLatitude) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.getCoordinates().hasLatitude())
        {
            return QString();
        }

        return QLocale().toString(m_gpsData.getCoordinates().lat(), 'g', 7);
    }
    else if ((column == ColumnLongitude) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.getCoordinates().hasLongitude())
        {
            return QString();
        }

        return QLocale().toString(m_gpsData.getCoordinates().lon(), 'g', 7);
    }
    else if ((column == ColumnAltitude) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.getCoordinates().hasAltitude())
        {
            return QString();
        }

        return QLocale().toString(m_gpsData.getCoordinates().alt(), 'g', 7);
    }
    else if (column == ColumnAccuracy)
    {
        if (role == Qt::DisplayRole)
        {
            if (m_gpsData.hasDop())
            {
                return i18n("DOP: %1", m_gpsData.getDop());
            }

            if (m_gpsData.hasFixType())
            {
                return i18n("Fix: %1d", m_gpsData.getFixType());
            }

            if (m_gpsData.hasNSatellites())
            {
                return i18n("#Sat: %1", m_gpsData.getNSatellites());
            }
        }
        else if (role == Qt::BackgroundRole)
        {
            const int warningLevel = getWarningLevelFromGPSDataContainer(m_gpsData);

            switch (warningLevel)
            {
                case 1:
                    return QBrush(Qt::green);

                case 2:
                    return QBrush(Qt::yellow);

                case 3:
                    // orange
                    return QBrush(QColor(0xff, 0x80, 0x00));

                case 4:
                    return QBrush(Qt::red);

                default:
                    break;
            }
        }
    }
    else if ((column == ColumnDOP) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.hasDop())
        {
            return QString();
        }

        return QString::number(m_gpsData.getDop());
    }
    else if ((column == ColumnFixType) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.hasFixType())
        {
            return QString();
        }

        return i18n("%1d", m_gpsData.getFixType());
    }
    else if ((column == ColumnNSatellites) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.hasNSatellites())
        {
            return QString();
        }

        return QString::number(m_gpsData.getNSatellites());
    }
    else if ((column == ColumnSpeed) && (role == Qt::DisplayRole))
    {
        if (!m_gpsData.hasSpeed())
        {
            return QString();
        }

        return QLocale().toString(m_gpsData.getSpeed());
    }
    else if ((column == ColumnStatus) && (role == Qt::DisplayRole))
    {
        if (m_dirty || m_tagListDirty)
        {
            return i18n("Modified");
        }

        return QString();
    }
    else if ((column == ColumnTags) && (role == Qt::DisplayRole))
    {
        if (!m_tagList.isEmpty())
        {

            QString myTagsList;

            for (int i = 0 ; i < m_tagList.count() ; ++i)
            {
                QString myTag;

                for (int j = 0 ; j < m_tagList[i].count() ; ++j)
                {
                    myTag.append(QLatin1Char('/') + m_tagList[i].at(j).tagName);

                    if (j == 0)
                    {
                        myTag.remove(0, 1);
                    }
                }

                if (!myTagsList.isEmpty())
                {
                    myTagsList.append(QLatin1String(", "));
                }

                myTagsList.append(myTag);
            }

            return myTagsList;
        }

        return QString();
    }

    return QVariant();
}

void GPSItemContainer::setCoordinates(const GeoCoordinates& newCoordinates)
{
    m_gpsData.setCoordinates(newCoordinates);
    m_dirty = true;

    emitDataChanged();
}

void GPSItemContainer::setModel(GPSItemModel* const model)
{
    m_model = model;
}

void GPSItemContainer::emitDataChanged()
{
    if (m_model)
    {
        m_model->itemChanged(this);
    }
}

void GPSItemContainer::setHeaderData(GPSItemModel* const model)
{
    model->setColumnCount(ColumnGPSItemContainerCount);
    model->setHeaderData(ColumnThumbnail,   Qt::Horizontal, i18n("Thumbnail"),      Qt::DisplayRole);
    model->setHeaderData(ColumnFilename,    Qt::Horizontal, i18n("Filename"),       Qt::DisplayRole);
    model->setHeaderData(ColumnDateTime,    Qt::Horizontal, i18n("Date and time"),  Qt::DisplayRole);
    model->setHeaderData(ColumnLatitude,    Qt::Horizontal, i18n("Latitude"),       Qt::DisplayRole);
    model->setHeaderData(ColumnLongitude,   Qt::Horizontal, i18n("Longitude"),      Qt::DisplayRole);
    model->setHeaderData(ColumnAltitude,    Qt::Horizontal, i18n("Altitude"),       Qt::DisplayRole);
    model->setHeaderData(ColumnAccuracy,    Qt::Horizontal, i18n("Accuracy"),       Qt::DisplayRole);
    model->setHeaderData(ColumnDOP,         Qt::Horizontal, i18n("DOP"),            Qt::DisplayRole);
    model->setHeaderData(ColumnFixType,     Qt::Horizontal, i18n("Fix type"),       Qt::DisplayRole);
    model->setHeaderData(ColumnNSatellites, Qt::Horizontal, i18n("# satellites"),   Qt::DisplayRole);
    model->setHeaderData(ColumnSpeed,       Qt::Horizontal, i18n("Speed"),          Qt::DisplayRole);
    model->setHeaderData(ColumnStatus,      Qt::Horizontal, i18n("Status"),         Qt::DisplayRole);
    model->setHeaderData(ColumnTags,        Qt::Horizontal, i18n("Tags"),           Qt::DisplayRole);
}

bool GPSItemContainer::lessThan(const GPSItemContainer* const otherItem, const int column) const
{
    switch (column)
    {
        case ColumnThumbnail:
            return false;

        case ColumnFilename:
            return (m_url < otherItem->m_url);

        case ColumnDateTime:
            return (m_dateTime < otherItem->m_dateTime);

        case ColumnAltitude:
        {
            if (!m_gpsData.hasAltitude())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasAltitude())
            {
                return true;
            }

            return (m_gpsData.getCoordinates().alt() < otherItem->m_gpsData.getCoordinates().alt());
        }

        case ColumnNSatellites:
        {
            if (!m_gpsData.hasNSatellites())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasNSatellites())
            {
                return true;
            }

            return (m_gpsData.getNSatellites() < otherItem->m_gpsData.getNSatellites());
        }

        case ColumnAccuracy:
        {
            const int myWarning    = getWarningLevelFromGPSDataContainer(m_gpsData);
            const int otherWarning = getWarningLevelFromGPSDataContainer(otherItem->m_gpsData);

            if (myWarning < 0)
            {
                return false;
            }

            if (otherWarning < 0)
            {
                return true;
            }

            if (myWarning != otherWarning)
            {
                return (myWarning < otherWarning);
            }

            // TODO: this may not be the best way to sort images with equal warning levels
            //       but it works for now

            if (m_gpsData.hasDop() != otherItem->m_gpsData.hasDop())
            {
                return !m_gpsData.hasDop();
            }

            if (m_gpsData.hasDop() && otherItem->m_gpsData.hasDop())
            {
                return (m_gpsData.getDop() < otherItem->m_gpsData.getDop());
            }

            if (m_gpsData.hasFixType() != otherItem->m_gpsData.hasFixType())
            {
                return m_gpsData.hasFixType();
            }

            if (m_gpsData.hasFixType() && otherItem->m_gpsData.hasFixType())
            {
                return (m_gpsData.getFixType() > otherItem->m_gpsData.getFixType());
            }

            if (m_gpsData.hasNSatellites() != otherItem->m_gpsData.hasNSatellites())
            {
                return m_gpsData.hasNSatellites();
            }

            if (m_gpsData.hasNSatellites() && otherItem->m_gpsData.hasNSatellites())
            {
                return (m_gpsData.getNSatellites() > otherItem->m_gpsData.getNSatellites());
            }

            return false;
        }

        case ColumnDOP:
        {
            if (!m_gpsData.hasDop())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasDop())
            {
                return true;
            }

            return (m_gpsData.getDop() < otherItem->m_gpsData.getDop());
        }

        case ColumnFixType:
        {
            if (!m_gpsData.hasFixType())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasFixType())
            {
                return true;
            }

            return (m_gpsData.getFixType() < otherItem->m_gpsData.getFixType());
        }

        case ColumnSpeed:
        {
            if (!m_gpsData.hasSpeed())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasSpeed())
            {
                return true;
            }

            return (m_gpsData.getSpeed() < otherItem->m_gpsData.getSpeed());
        }

        case ColumnLatitude:
        {
            if (!m_gpsData.hasCoordinates())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasCoordinates())
            {
                return true;
            }

            return (m_gpsData.getCoordinates().lat() < otherItem->m_gpsData.getCoordinates().lat());
        }

        case ColumnLongitude:
        {
            if (!m_gpsData.hasCoordinates())
            {
                return false;
            }

            if (!otherItem->m_gpsData.hasCoordinates())
            {
                return true;
            }

            return (m_gpsData.getCoordinates().lon() < otherItem->m_gpsData.getCoordinates().lon());
        }

        case ColumnStatus:
        {
            return (m_dirty && !otherItem->m_dirty);
        }

        default:
        {
            return false;
        }
    }
}

SaveProperties GPSItemContainer::saveProperties() const
{
    SaveProperties p;

    // do we have gps information?

    if (m_gpsData.hasCoordinates())
    {
        p.shouldWriteCoordinates = true;
        p.latitude               = m_gpsData.getCoordinates().lat();
        p.longitude              = m_gpsData.getCoordinates().lon();

        if (m_gpsData.hasAltitude())
        {
            p.shouldWriteAltitude = true;
            p.altitude            = m_gpsData.getCoordinates().alt();
        }
        else
        {
            p.shouldRemoveAltitude = true;
        }
    }
    else
    {
        p.shouldRemoveCoordinates = true;
    }

    return p;
}

QString GPSItemContainer::saveChanges()
{
    SaveProperties p = saveProperties();

    QString returnString;

    // first try to write the information to the image file

    bool success1 = true;
    QScopedPointer<DMetadata> meta(getMetadataForFile());

    if (!meta)
    {
        // TODO: more verbosity!

        returnString = i18n("Failed to open file.");
        success1     = false;
    }
    else
    {
        if (m_saveGPS && p.shouldWriteCoordinates)
        {
            if (p.shouldWriteAltitude)
            {
                success1 = meta->setGPSInfo(p.altitude, p.latitude, p.longitude);
            }
            else
            {
                success1 = meta->setGPSInfo(nullptr,
                                            p.latitude, p.longitude);
            }

            // write all other GPS information here too

            if (success1 && m_gpsData.hasSpeed())
            {
                success1 = setExifXmpTagDataVariant(meta.data(),
                                                    "Exif.GPSInfo.GPSSpeedRef",
                                                    "Xmp.exif.GPSSpeedRef",
                                                    QVariant(QLatin1String("K")));

                if (success1)
                {
                    const qreal speedInMetersPerSecond   = m_gpsData.getSpeed();

                    // km/h = 0.001 * m / ( s * 1/(60*60) ) = 3.6 * m/s

                    const qreal speedInKilometersPerHour = 3.6 * speedInMetersPerSecond;
                    success1                             = setExifXmpTagDataVariant(meta.data(), "Exif.GPSInfo.GPSSpeed", "Xmp.exif.GPSSpeed", QVariant(speedInKilometersPerHour));
                }
            }

            if (success1 && m_gpsData.hasNSatellites())
            {
                /**
                 * @todo According to the EXIF 2.2 spec, GPSSatellites is a free form field which can either hold only the
                 * number of satellites or more details about each satellite used. For now, we just write
                 * the number of satellites. Are we using the correct format for the number of satellites here?
                 */
                success1 = setExifXmpTagDataVariant(meta.data(),
                                                    "Exif.GPSInfo.GPSSatellites", "Xmp.exif.GPSSatellites",
                                                    QVariant(QString::number(m_gpsData.getNSatellites())));
            }

            if (success1 && m_gpsData.hasFixType())
            {
                success1 = setExifXmpTagDataVariant(meta.data(),
                                                    "Exif.GPSInfo.GPSMeasureMode", "Xmp.exif.GPSMeasureMode",
                                                    QVariant(QString::number(m_gpsData.getFixType())));
            }

            // write DOP

            if (success1 && m_gpsData.hasDop())
            {
                success1 = setExifXmpTagDataVariant(meta.data(),
                                                    "Exif.GPSInfo.GPSDOP",
                                                    "Xmp.exif.GPSDOP",
                                                    QVariant(m_gpsData.getDop()));
            }


            if (!success1)
            {
                returnString = i18n("Failed to add GPS info to image.");
            }
        }

        if (m_saveGPS && p.shouldRemoveCoordinates)
        {
            // TODO: remove only the altitude if requested

            success1 = meta->removeGPSInfo();

            if (!success1)
            {
                returnString = i18n("Failed to remove GPS info from image");
            }
        }

        if (m_saveTags && !m_tagList.isEmpty() && (m_writeXmpTags || m_writeMetaLoc))
        {
            if (m_writeXmpTags)
            {
                QStringList tagSeq = meta->getXmpTagStringSeq("Xmp.digiKam.TagsList", false);

                for (int i = 0 ; i < m_tagList.count() ; ++i)
                {
                    QList<TagData> currentTagList = m_tagList[i];
                    QString tag;

                    for (int j = 0 ; j < currentTagList.count() ; ++j)
                    {
                        if (currentTagList[j].tipName != QLatin1String("{Country code}"))
                        {
                            tag.append(QLatin1Char('/') + currentTagList[j].tagName);
                        }
                    }

                    tag.remove(0, 1);

                    if (!tagSeq.contains(tag))
                    {
                        tagSeq.append(tag);
                    }
                }

                bool success2 = meta->setXmpTagStringSeq("Xmp.digiKam.TagsList", tagSeq);

                if (!success2)
                {
                    returnString = i18n("Failed to save tags to file.");
                }

                success2 = meta->setXmpTagStringSeq("Xmp.dc.subject", tagSeq);

                if (!success2)
                {
                    returnString = i18n("Failed to save tags to file.");
                }
            }

            if (m_writeMetaLoc)
            {
                IptcCoreLocationInfo locationInfo;

                for (int i = 0 ; i < m_tagList.count() ; ++i)
                {
                    QList<TagData> currentTagList = m_tagList[i];
                    QString tag;

                    for (int j = 0 ; j < currentTagList.count() ; ++j)
                    {
                        setLocationInfo(currentTagList[j], locationInfo);
                    }
                }

                bool success3 = meta->setIptcCoreLocation(locationInfo);

                if (!success3)
                {
                    returnString = i18n("Failed to save tags to file.");
                }
            }
        }
    }

    if (success1)
    {
        if (m_saveGPS || (m_saveTags && !m_tagList.isEmpty() && (m_writeXmpTags || m_writeMetaLoc)))
        {
            success1 = meta->save(m_url.toLocalFile());
        }

        if (!success1)
        {
            returnString = i18n("Unable to save changes to file");
        }
        else
        {
            m_dirty        = false;
            m_savedState   = m_gpsData;
            m_tagListDirty = false;
            m_savedTagList = m_tagList;
        }
    }

    if (returnString.isEmpty())
    {
        // mark all changes as not dirty and tell the model:

        emitDataChanged();
    }

    return returnString;
}

/**
 * @brief Restore the gps data to @p container. Sets m_dirty to false if container equals savedState.
 */
void GPSItemContainer::restoreGPSData(const GPSDataContainer& container)
{
    m_dirty   = !(container == m_savedState);
    m_gpsData = container;

    emitDataChanged();
}

void GPSItemContainer::restoreRGTagList(const QList<QList<TagData> >& tagList)
{
    // TODO: override == operator

    if (tagList.count() != m_savedTagList.count())
    {
        m_tagListDirty = true;
    }
    else
    {
        for (int i = 0 ; i < tagList.count() ; ++i)
        {
            bool foundNotEqual = false;

            if (tagList[i].count() != m_savedTagList[i].count())
            {
                m_tagListDirty = true;
                break;
            }

            for (int j = 0 ; j < tagList[i].count() ; ++j)
            {
                if (tagList[i].at(j).tagName != m_savedTagList[i].at(j).tagName)
                {
                    foundNotEqual = true;
                    break;
                }
            }

            if (foundNotEqual)
            {
                m_tagListDirty = true;
                break;
            }
        }
    }

    m_tagList = tagList;

    emitDataChanged();
}

bool GPSItemContainer::isDirty() const
{
    return m_dirty;
}

QUrl GPSItemContainer::url() const
{
    return m_url;
}

QDateTime GPSItemContainer::dateTime() const
{
    return m_dateTime;
}

GeoCoordinates GPSItemContainer::coordinates() const
{
    return m_gpsData.getCoordinates();
}

GPSDataContainer GPSItemContainer::gpsData() const
{
    return m_gpsData;
}

void GPSItemContainer::setGPSData(const GPSDataContainer& container)
{
    m_gpsData = container;
    m_dirty   = true;

    emitDataChanged();
}

void GPSItemContainer::setTagList(const QList<QList<TagData> >& externalTagList)
{
    m_tagList      = externalTagList;
    m_tagListDirty = true;

    emitDataChanged();
}

bool GPSItemContainer::isTagListDirty() const
{
    return m_tagListDirty;
}

QList<QList<TagData> > GPSItemContainer::getTagList() const
{
    return m_tagList;
}

void GPSItemContainer::setLocationInfo(const TagData& tagData, IptcCoreLocationInfo& locationInfo)
{
    if      (tagData.tipName == QLatin1String("{Country}"))
    {
        locationInfo.country       = tagData.tagName;
    }
    else if (tagData.tipName == QLatin1String("{Country code}"))
    {
        locationInfo.countryCode   = tagData.tagName;
    }
    else if (tagData.tipName == QLatin1String("{State}"))
    {
        locationInfo.provinceState = tagData.tagName;
    }
    else if (locationInfo.provinceState.isEmpty() &&
             (tagData.tipName == QLatin1String("{State district}")))
    {
        locationInfo.provinceState = tagData.tagName;
    }
    else if (locationInfo.provinceState.isEmpty() &&
             (tagData.tipName == QLatin1String("{County}")))
    {
        locationInfo.provinceState = tagData.tagName;
    }
    else if (tagData.tipName == QLatin1String("{City}"))
    {
        locationInfo.city          = tagData.tagName;
    }
    else if (locationInfo.city.isEmpty() &&
             (tagData.tipName == QLatin1String("{City district}")))
    {
        locationInfo.city          = tagData.tagName;
    }
    else if (locationInfo.city.isEmpty() &&
             (tagData.tipName == QLatin1String("{Suburb}")))
    {
        locationInfo.city          = tagData.tagName;
    }
    else if (locationInfo.city.isEmpty() &&
             (tagData.tipName == QLatin1String("{Town}")))
    {
        locationInfo.city          = tagData.tagName;
    }
    else if (locationInfo.city.isEmpty() &&
             (tagData.tipName == QLatin1String("{Village}")))
    {
        locationInfo.city          = tagData.tagName;
    }
    else if (locationInfo.city.isEmpty() &&
             (tagData.tipName == QLatin1String("{Hamlet}")))
    {
        locationInfo.city          = tagData.tagName;
    }
    else if (tagData.tipName == QLatin1String("{Street}"))
    {
        locationInfo.location      = tagData.tagName;
    }
}

} // namespace Digikam
