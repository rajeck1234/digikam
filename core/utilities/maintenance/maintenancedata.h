/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-02-20
 * Description : Synchronized container for maintenance data.
 *
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MAINTENANCE_DATA_H
#define DIGIKAM_MAINTENANCE_DATA_H

// Qt includes

#include <QList>

// Local includes

#include "iteminfo.h"
#include "identity.h"

namespace Digikam
{

class ImageQualitySettings;

class MaintenanceData
{
public:

    explicit MaintenanceData();
    ~MaintenanceData();

    void      setImageIds(const QList<qlonglong>& ids);
    void      setThumbnailIds(const QList<int>& ids);
    void      setImagePaths(const QList<QString>& paths);
    void      setItemInfos(const QList<ItemInfo>& infos);
    void      setIdentities(const QList<Identity>& identities);
    void      setSimilarityImageIds(const QList<qlonglong>& ids);

    void      setRebuildAllFingerprints(bool b);

    qlonglong getImageId()                const;
    int       getThumbnailId()            const;
    QString   getImagePath()              const;
    ItemInfo getItemInfo()                const;
    Identity  getIdentity()               const;
    qlonglong getSimilarityImageId()      const;

    bool      getRebuildAllFingerprints() const;

private:

    // Disable
    MaintenanceData(const MaintenanceData&)            = delete;
    MaintenanceData& operator=(const MaintenanceData&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_MAINTENANCE_DATA_H
