/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Track file reader
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TRACK_READER_H
#define DIGIKAM_TRACK_READER_H

// Qt includes

#include <QXmlStreamReader>

// local includes

#include "trackmanager.h"
#include "digikam_export.h"

class TestTracks;

namespace Digikam
{

class DIGIKAM_EXPORT TrackReader : public QObject
{
    Q_OBJECT

public:

    class TrackReadResult
    {
    public:

        explicit TrackReadResult()
          : track    (),
            isValid  (false),
            loadError()
        {
        }

        TrackManager::Track            track;
        bool                           isValid;
        QString                        loadError;

        typedef QList<TrackReadResult> List;
    };

public:

    explicit TrackReader(TrackReadResult* const dataTarget);
    ~TrackReader()                                          override;

    static TrackReadResult loadTrackFile(const QUrl& url);
    static QDateTime ParseTime(const QString& tstring);

private:

    void parseTrack(QXmlStreamReader& xml);
    explicit TrackReader(QObject*);

private:

    class Private;
    const QScopedPointer<Private> d;

    friend class ::TestTracks;
};

} // namespace Digikam

#endif // DIGIKAM_TRACK_READER_H
