/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-12
 * Description : Abstract backend class for reverse geocoding.
 *
 * SPDX-FileCopyrightText: 2010 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2010 by Gabriel Voicu <ping dot gabi at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BACKEND_RG_H
#define DIGIKAM_BACKEND_RG_H

// local includes

#include "gpsitemcontainer.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RGBackend : public QObject
{
    Q_OBJECT

public:

    explicit RGBackend(QObject* const parent);
    RGBackend();
    ~RGBackend() override;

    virtual void callRGBackend(const QList<RGInfo>&, const  QString&) = 0;
    virtual QString getErrorMessage();
    virtual QString backendName();
    virtual void cancelRequests()                                     = 0;

Q_SIGNALS:

    /**
     * @brief Emitted whenever some items are ready
     */
    void signalRGReady(QList<RGInfo>&);
};

} // namespace Digikam

#endif // DIGIKAM_BACKEND_RG_H
