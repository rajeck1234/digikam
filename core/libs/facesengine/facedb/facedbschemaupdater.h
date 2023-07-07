/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-05-29
 * Description : Face database schema updater
 *
 * SPDX-FileCopyrightText: 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2020 by Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FACE_DB_SCHEMA_UPDATER_H
#define DIGIKAM_FACE_DB_SCHEMA_UPDATER_H

// Local includes

#include "collectionscannerobserver.h"

namespace Digikam
{

class FaceDbAccess;

class FaceDbSchemaUpdater
{
public:

    static int schemaVersion();

public:

    explicit FaceDbSchemaUpdater(FaceDbAccess* const dbAccess);
    ~FaceDbSchemaUpdater();

    bool update();
    void setObserver(InitializationObserver* const observer);

private:

    bool startUpdates();
    bool makeUpdates();
    bool createDatabase();
    bool createTables();
    bool createIndices();
    bool createTriggers();
    bool updateV1ToV2();
    bool updateV2ToV3();
    bool updateV3ToV4();

private:

    // Disable
    FaceDbSchemaUpdater(const FaceDbSchemaUpdater&)            = delete;
    FaceDbSchemaUpdater& operator=(const FaceDbSchemaUpdater&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_FACE_DB_SCHEMA_UPDATER_H
