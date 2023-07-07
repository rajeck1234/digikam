/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : Core database abstract backend.
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CORE_DB_BACKEND_H
#define DIGIKAM_CORE_DB_BACKEND_H

// Local includes

#include "digikam_export.h"
#include "dbenginebackend.h"
#include "coredbchangesets.h"

namespace Digikam
{

class CoreDbSchemaUpdater;
class CoreDbWatch;
class CoreDbBackendPrivate;

class DIGIKAM_DATABASE_EXPORT CoreDbBackend : public BdEngineBackend
{
    Q_OBJECT

public:

    explicit CoreDbBackend(DbEngineLocking* const locking,
                           const QString& backendName = QLatin1String("digikamDatabase-"));
    ~CoreDbBackend() override;

    /**
     * Sets the global database watch
     */
    void setCoreDbWatch(CoreDbWatch* watch);

    /**
     * Initialize the database schema to the current version,
     * carry out upgrades if necessary.
     * Shall only be called from the thread that called open().
     */
    bool initSchema(CoreDbSchemaUpdater* updater);

    /**
     * Notify all listeners of the changeset
     */
    void recordChangeset(const ImageChangeset& changeset);
    void recordChangeset(const ImageTagChangeset& changeset);
    void recordChangeset(const CollectionImageChangeset& changeset);
    void recordChangeset(const AlbumChangeset& changeset);
    void recordChangeset(const TagChangeset& changeset);
    void recordChangeset(const AlbumRootChangeset& changeset);
    void recordChangeset(const SearchChangeset& changeset);

private:

    // Disable
    explicit CoreDbBackend(QObject*) = delete;

    Q_DECLARE_PRIVATE(CoreDbBackend)
};

} // namespace Digikam

#endif // DIGIKAM_CORE_DB_BACKEND_H
