/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-22-01
 * Description : batch sync pictures metadata with database
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_METADATA_SYNCHRONIZER_H
#define DIGIKAM_METADATA_SYNCHRONIZER_H

// Qt includes

#include <QObject>

// Local includes

#include "album.h"
#include "iteminfo.h"
#include "maintenancetool.h"

namespace Digikam
{

class MetadataSynchronizer : public MaintenanceTool
{
    Q_OBJECT

public:

    enum SyncDirection
    {
        WriteFromDatabaseToFile = 0,
        ReadFromFileToDatabase
    };

public:

    /** Constructor which sync all pictures metadata from an Albums list. If list is empty, whole Albums collection is processed.
     */
    explicit MetadataSynchronizer(const AlbumList& list = AlbumList(), SyncDirection direction = WriteFromDatabaseToFile, ProgressItem* const parent = nullptr);

    /** Constructor which sync all pictures metadata from an Images list
     */
    explicit MetadataSynchronizer(const ItemInfoList& list, SyncDirection = WriteFromDatabaseToFile, ProgressItem* const parent = nullptr);

    void setTagsOnly(bool value);

    ~MetadataSynchronizer() override;

    void setUseMultiCoreCPU(bool b) override;

private Q_SLOTS:

    void slotStart() override;
    void slotParseAlbums();
    void slotAlbumParsed(const ItemInfoList&);
    void slotAdvance();
    void slotOneAlbumIsComplete();
    void slotCancel() override;

private:

    void init(SyncDirection direction);
    void parseList();
    void parsePicture();
    void processOneAlbum();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_METADATA_SYNCHRONIZER_H
