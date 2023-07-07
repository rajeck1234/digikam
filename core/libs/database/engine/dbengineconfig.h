/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-27
 * Description : Database engine configuration
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_ENGINE_CONFIG_H
#define DIGIKAM_DB_ENGINE_CONFIG_H

// Local includes

#include "digikam_export.h"
#include "dbengineconfigsettings.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineConfig
{
public:

    static bool                   checkReadyForUse();
    static QString                errorMessage();
    static DbEngineConfigSettings element(const QString& databaseType);
};

} // namespace Digikam

#endif // DIGIKAM_DB_ENGINE_CONFIG_H
