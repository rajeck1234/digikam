/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-11-01
 * Description : Access item position stored in database.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Patrick Spendrin <ps_ml at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_POSITION_H
#define DIGIKAM_ITEM_POSITION_H

// Qt includes

#include <QString>
#include <QSharedDataPointer>
#include <QSharedData>

// Local includes

#include "digikam_export.h"
#include "coredbfields.h"

namespace Digikam
{

class CoreDbAccess;
class ItemPositionPriv;

class DIGIKAM_DATABASE_EXPORT ItemPosition
{

public:

    /**
     * Creates a null ItemPosition object
     */
    ItemPosition();

    /**
     * Creates an ItemPosition object for the given image.
     * The information is read from the database.
     */
    explicit ItemPosition(qlonglong imageId);
    ItemPosition(const CoreDbAccess& access, qlonglong imageId);

    ItemPosition(const ItemPosition& other);
    ~ItemPosition();

    ItemPosition& operator=(const ItemPosition& other);

    bool isNull()                const;
    /**
     * An object is empty if no entry exists in the ItemPosition
     * table for the referenced image, or if the object is null.
     * An empty object is empty even if values have been set;
     * it becomes not empty after calling apply().
     */
    bool isEmpty()               const;

    /**
     * Returns latitude/longitude in the format as described by
     * the XMP specification as "GPSCoordinate":
     * A Text value in the form ?DDD,MM,SSk? or ?DDD,MM.mmk?.
     * This provides lossless storage.
     */
    QString latitude()           const;
    QString longitude()          const;

    /**
     * Returns latitude/longitude as a double in degrees.
     * North and East have a positive sign, South and West negative.
     * This provides high precision, with the usual floating point
     * concerns, and possible problems finding the exact text form when
     * converting _back_ to fractions.
     */
    double latitudeNumber()      const;
    double longitudeNumber()     const;

    /**
     * Returns the latitude/longitude in a user-presentable version,
     * in the form "30°45'55.123'' East"
     */
    QString latitudeFormatted()  const;
    QString longitudeFormatted() const;

    /**
     * Returns latitude/longitude as user-presentable numbers.
     * This means that degrees and minutes are integer, the seconds fractional.
     * Direction reference is 'N'/'S', 'E'/'W' resp.
     * This is for the purpose of presenting to the user, there are no guarantees on precision.
     * Returns true if the values have been changed.
     */
    bool latitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference);
    bool longitudeUserPresentableNumbers(int* degrees, int* minutes, double* seconds, char* directionReference);

    /**
     * The altitude in meters
     */
    double altitude()            const;

    /**
     * Returns the altitude formatted in a user-presentable way in the form "43.45m"
     */
    QString altitudeFormatted()  const;
    double orientation()         const;
    double tilt()                const;
    double roll()                const;
    double accuracy()            const;
    QString description()        const;

    bool hasCoordinates()        const;
    bool hasAltitude()           const;
    bool hasOrientation()        const;
    bool hasTilt()               const;
    bool hasRoll()               const;
    bool hasAccuracy()           const;

    /**
     * Sets the latitude/longitude from the GPSCoordinate string as described by XMP.
     * Returns true if the format is accepted.
     */
    bool setLatitude(const QString& latitude);
    bool setLongitude(const QString& longitude);

    /**
     * Sets the latitude/longitude from a double floating point number,
     * as described for latitudeNumber() above.
     * Returns true if the value is valid and accepted.
     */
    bool setLatitude(double latitudeNumber);
    bool setLongitude(double longitudeNumber);

    /**
     * Set the altitude in meters
     */
    void setAltitude(double altitude);
    void setOrientation(double orientation);
    void setTilt(double tilt);
    void setRoll(double roll);
    void setAccuracy(double accuracy);
    void setDescription(const QString& description);

    /**
     * Apply all changes made to this object.
     * (Also called from destructor)
     */
    void apply();

    /**
     * Removes the whole data set for the referenced image
     * from the database.
     * This object and any ItemPosition object created later
     * will be empty.
     */
    void remove();

    /**
     * Removes the altitude for the referenced image
     * from the database.
     */
    void removeAltitude();

private:

    QSharedDataPointer<ItemPositionPriv> d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_POSITION_H
