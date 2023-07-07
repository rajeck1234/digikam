/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file loading and managing
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TRACK_MANAGER_H
#define DIGIKAM_TRACK_MANAGER_H

// Qt includes

#include <QColor>
#include <QDateTime>
#include <QUrl>

// local includes

#include "geoifacetypes.h"
#include "geocoordinates.h"
#include "digikam_export.h"

class TestGPXParsing;

namespace Digikam
{

class DIGIKAM_EXPORT TrackManager : public QObject
{
    Q_OBJECT

public:

    /// NOTE: we assume here that we will never load more than uint32_max tracks.
    typedef quint32 Id;

    enum ChangeFlag
    {
        ChangeTrackPoints = 1,
        ChangeMetadata    = 2,

        ChangeRemoved     = 4,
        ChangeAdd         = ChangeTrackPoints | ChangeMetadata
    };

    typedef QPair<Id, ChangeFlag> TrackChanges;

public:

    class  TrackPoint
    {
    public:

        explicit TrackPoint()
          : dateTime    (),
            coordinates (),
            nSatellites (-1),
            hDop        (-1),
            pDop        (-1),
            fixType     (-1),
            speed       (-1)
        {
        }

        static bool EarlierThan(const TrackPoint& a, const TrackPoint& b);

    public:

        QDateTime                 dateTime;
        GeoCoordinates            coordinates;
        int                       nSatellites;
        qreal                     hDop;
        qreal                     pDop;
        int                       fixType;
        qreal                     speed;

        typedef QList<TrackPoint> List;
    };

    // -------------------------------------

    class Track
    {
    public:

        enum Flags
        {
            FlagVisible = 1,
            FlagDefault = FlagVisible
        };

    public:

        explicit Track()
          : url     (),
            points  (),
            id      (0),
            color   (Qt::red),
            flags   (FlagDefault)
        {
            qRegisterMetaType<TrackChanges>("TrackChanges");
        }

        QUrl                 url;
        QList<TrackPoint>    points;

        /// 0 means no track id assigned yet
        Id                   id;
        QColor               color;
        Flags                flags;

        typedef QList<Track> List;
    };

public:

    explicit TrackManager(QObject* const parent = nullptr);
    ~TrackManager()                                   override;

    void loadTrackFiles(const QList<QUrl>& urls);
    QList<QPair<QUrl, QString> > readLoadErrors();
    void clear();

    const Track& getTrack(const int index)      const;
    Track::List getTrackList()                  const;
    int trackCount()                            const;

    quint64 getNextFreeTrackId();
    Track   getTrackById(const quint64 trackId) const;
    QColor  getNextFreeTrackColor();

    void setVisibility(const bool value);
    bool getVisibility()                        const;

Q_SIGNALS:

    void signalTrackFilesReadyAt(const int startIndex, const int endIndex);
    void signalAllTrackFilesReady();
    void signalTracksChanged(const QList<TrackManager::TrackChanges>& trackChanges);
    void signalVisibilityChanged(const bool newValue);

private Q_SLOTS:

    void slotTrackFilesReadyAt(int beginIndex, int endIndex);
    void slotTrackFilesFinished();

private:

    class Private;
    const QScopedPointer<Private> d;
};

} // namespace Digikam

#endif // DIGIKAM_TRACK_MANAGER_H
