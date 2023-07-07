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

#include "backend-rg.h"

namespace Digikam
{

/**
 * @class RGBackend
 *
 * @brief This class is a base class for Open Street Map and Geonames backends.
 */

/**
 * Constructor
 */
RGBackend::RGBackend(QObject* const parent)
    : QObject(parent)
{
}

/**
 * Destructor
 */
RGBackend::~RGBackend()
{
}

QString RGBackend::getErrorMessage()
{
    return QString();
}

QString RGBackend::backendName()
{
    return QString();
}

} // namespace Digikam
