/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-07-26
 * Description : common items needed for web services
 *
 * SPDX-FileCopyrightText: 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_WS_ITEM_H
#define DIGIKAM_WS_ITEM_H

// Qt includes

#include <QString>
#include <QStringList>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSAlbum
{

public:

    explicit WSAlbum()
      : parentID(QLatin1String("")),
        isRoot(true),
        description(QLatin1String("")),
        url(QLatin1String("")),
        uploadable(true)
    {
    }

    /**
     * This method is used by derived class of WSAblum,
     * to set the attributes inherited from WSAlbum, knowing
     * a WSAlbum.
     */
    void setBaseAlbum(const WSAlbum& album)
    {
        id          = album.id;
        parentID    = album.parentID;
        isRoot      = album.isRoot;
        title       = album.title;
        description = album.description;
        location    = album.location;
        url         = album.url;
        uploadable  = album.uploadable;
    }

    QString   id;
    QString   parentID;
    bool      isRoot;

    QString   title;
    QString   description;
    QString   location;
    QString   url;
    bool      uploadable;
};

/**
 * This class is used when parsing response of listAlbums().
 * It contains only the most important attributes of an album,
 * which is needed for further usage (e.g upload photos, create new album).
 */
class DIGIKAM_EXPORT AlbumSimplified
{

public:

    explicit AlbumSimplified()
      : uploadable(true)
    {
    }

    explicit AlbumSimplified(const QString& title)
      : title(title),
        uploadable(true)
    {
    }

    explicit AlbumSimplified(const QString& title, bool uploadable)
      : title(title),
        uploadable(uploadable)
    {
    }

public:

    QString     title;
    QStringList childrenIDs;
    bool        uploadable;
};

} // namespace Digikam

#endif // DIGIKAM_WS_ITEM_H
