/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-03-21
 * Description : a class to hold GPS information about an item.
 *
 * SPDX-FileCopyrightText: 2010-2014 by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_GPS_H
#define DIGIKAM_ITEM_GPS_H

// Qt includes

#include <QList>

// Local includes

#include "digikam_export.h"
#include "iteminfo.h"
#include "gpsitemcontainer.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ItemGPS : public GPSItemContainer
{

public:

    explicit ItemGPS(const ItemInfo& info);
    ~ItemGPS()            override;

    QString saveChanges() override;
    bool loadImageData()  override;

private:

    ItemInfo m_info;

private:

    Q_DISABLE_COPY(ItemGPS)
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_GPS_H
