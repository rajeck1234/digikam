/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-14-02
 * Description : interface to get item info from an albums list.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_INFO_ALBUMS_JOB_H
#define DIGIKAM_ITEM_INFO_ALBUMS_JOB_H

// Qt includes

#include <QObject>

// Local includes

#include "iteminfo.h"
#include "albummanager.h"

namespace Digikam
{

class ItemInfoAlbumsJob : public QObject
{
    Q_OBJECT

public:

    explicit ItemInfoAlbumsJob(QObject* const parent = nullptr);
    ~ItemInfoAlbumsJob() override;

    void allItemsFromAlbums(const AlbumList& albumsList);
    void stop();

Q_SIGNALS:

    void signalCompleted(const ItemInfoList& items);

private Q_SLOTS:

    void slotItemsInfo(const ItemInfoList&);
    void slotComplete();

private:

    void parseAlbum();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_INFO_ALBUMS_JOB_H
