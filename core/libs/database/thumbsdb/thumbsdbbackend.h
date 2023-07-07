/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Thumbnails database backend
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_THUMBS_DB_BACKEND_H
#define DIGIKAM_THUMBS_DB_BACKEND_H

// Local includes

#include "digikam_export.h"
#include "dbenginebackend.h"

namespace Digikam
{

class ThumbsDbSchemaUpdater;

class DIGIKAM_EXPORT ThumbsDbBackend : public BdEngineBackend
{
    Q_OBJECT

public:

    explicit ThumbsDbBackend(DbEngineLocking* const locking,
                             const QString& backendName = QLatin1String("thumbnailDatabase-"));
    ~ThumbsDbBackend() override;

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(ThumbsDbSchemaUpdater* const updater);

private:

    // Disabled
    explicit ThumbsDbBackend(QObject*) = delete;

    Q_DECLARE_PRIVATE(BdEngineBackend)
};

} // namespace Digikam

#endif // DIGIKAM_THUMBS_DB_BACKEND_H
