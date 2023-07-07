/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-30
 * Description : Base class for altitude lookup jobs
 *
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2011 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LOOKUP_ALTITUDE_H
#define DIGIKAM_LOOKUP_ALTITUDE_H

// Qt includes

#include <QObject>

// local includes

#include "digikam_export.h"
#include "geocoordinates.h"

namespace Digikam
{

class DIGIKAM_EXPORT LookupAltitude : public QObject
{
    Q_OBJECT

public:

    enum StatusEnum
    {
        StatusInProgress = 0,
        StatusSuccess    = 1,
        StatusCanceled   = 2,
        StatusError      = 3
    };
    Q_DECLARE_FLAGS(StatusAltitude, StatusEnum)

public:

    class  Request
    {
    public:

        explicit Request()
          : coordinates(),
            success    (false),
            data       ()
        {
        }

    public:

        GeoCoordinates         coordinates;
        bool                   success;
        QVariant               data;

        typedef QList<Request> List;
    };

public:

    explicit LookupAltitude(QObject* const parent);
    ~LookupAltitude() override;

    virtual QString backendName()                           const = 0;
    virtual QString backendHumanName()                      const = 0;

    virtual void addRequests(const Request::List& requests)       = 0;
    virtual Request::List getRequests()                     const = 0;
    virtual Request getRequest(const int index)             const = 0;

    virtual void startLookup()                                    = 0;
    virtual StatusAltitude getStatus()                      const = 0;
    virtual QString errorMessage()                          const = 0;
    virtual void cancel()                                         = 0;

Q_SIGNALS:

    void signalRequestsReady(const QList<int>& readyRequests);
    void signalDone();
};

} // namespace Digikam

Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::LookupAltitude::StatusAltitude)

#endif // DIGIKAM_LOOKUP_ALTITUDE_H
