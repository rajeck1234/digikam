/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-07-18
 * Description : batch face detection
 *
 * SPDX-FileCopyrightText: 2010      by Aditya Bhatt <adityabhatt1991 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FACES_DETECTOR_H
#define DIGIKAM_FACES_DETECTOR_H

// Qt includes

#include <QList>
#include <QObject>

// Local includes

#include "maintenancetool.h"
#include "iteminfo.h"

namespace Digikam
{

class DImg;
class FacePipelinePackage;
class FaceScanSettings;

class FacesDetector : public MaintenanceTool
{
    Q_OBJECT

public:

    enum InputSource
    {
        Albums = 0,
        Infos,
        Ids
    };

public:

    explicit FacesDetector(const FaceScanSettings& settings,
                           ProgressItem* const parent = nullptr);
    ~FacesDetector()                                                override;

private Q_SLOTS:

    void slotStart()                                                override;
    void slotContinueAlbumListing();
    void slotItemsInfo(const ItemInfoList&);
    void slotImagesSkipped(const QList<ItemInfo>&);
    void slotShowOneDetected(const FacePipelinePackage&);
    void slotDone()                                                 override;
    void slotCancel()                                               override;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FACES_DETECTOR_H
