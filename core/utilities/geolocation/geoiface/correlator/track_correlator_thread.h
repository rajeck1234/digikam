/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-19
 * Description : Thread for correlator for tracks and images
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TRACK_CORRELATOR_THREAD_H
#define DIGIKAM_TRACK_CORRELATOR_THREAD_H

// Qt includes

#include <QThread>

// Local includes

#include "track_correlator.h"

namespace Digikam
{

class TrackCorrelatorThread : public QThread
{
    Q_OBJECT

public:

    explicit TrackCorrelatorThread(QObject* const parent = nullptr);
    ~TrackCorrelatorThread() override;

public:

    TrackCorrelator::Correlation::List  itemsToCorrelate;
    TrackCorrelator::CorrelationOptions options;
    TrackManager::Track::List           fileList;
    bool                                doCancel;
    bool                                canceled;

protected:

    void run() override;

Q_SIGNALS:

    void signalItemsCorrelated(const Digikam::TrackCorrelator::Correlation::List& correlatedItems);
};

} // namespace Digikam

#endif // DIGIKAM_TRACK_CORRELATOR_THREAD_H
