/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-10-22
 * Description : a widget to filter album contents by geolocation
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_GEO_LOCATION_FILTER_H
#define DIGIKAM_GEO_LOCATION_FILTER_H

// Qt includes

#include <QComboBox>

// Local includes

#include "itemfiltersettings.h"

namespace Digikam
{

class GeolocationFilter : public QComboBox
{
    Q_OBJECT

public:

    explicit GeolocationFilter(QWidget* const parent);
    ~GeolocationFilter() override;

    void setGeolocationFilter(const ItemFilterSettings::GeolocationCondition& condition);
    ItemFilterSettings::GeolocationCondition geolocationFilter() const;

Q_SIGNALS:

    void signalFilterChanged(const ItemFilterSettings::GeolocationCondition& condition);

private Q_SLOTS:

    void slotFilterChanged();
};

} // namespace Digikam

#endif // DIGIKAM_GEO_LOCATION_FILTER_H
