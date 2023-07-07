/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : finger-prints generator
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FINGERPRINTS_GENERATOR_H
#define DIGIKAM_FINGERPRINTS_GENERATOR_H

// Qt includes

#include <QObject>

// Local includes

#include "album.h"
#include "maintenancetool.h"

class QImage;

namespace Digikam
{

class FingerPrintsGenerator : public MaintenanceTool
{
    Q_OBJECT

public:

    /** Constructor using AlbumList as argument. If list is empty, whole Albums collection is processed.
     */
    explicit FingerPrintsGenerator(const bool rebuildAll, const AlbumList& list = AlbumList(), ProgressItem* const parent = nullptr);
    ~FingerPrintsGenerator() override;

    void setUseMultiCoreCPU(bool b) override;

private:

    void processOne();

private Q_SLOTS:

    void slotStart()                override;
    void slotDone()                 override;
    void slotCancel()               override;
    void slotAdvance(const QImage&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FINGERPRINTS_GENERATOR_H
