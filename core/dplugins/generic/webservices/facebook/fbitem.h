/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-26
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FB_ITEM_H
#define DIGIKAM_FB_ITEM_H

// Qt includes

#include <QString>

// Local includes

#include "wsitem.h"

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

class FbUser
{
public:

    FbUser()
    {
        uploadPerm = false;
    }

    void clear()
    {
        id.clear();
        name.clear();
        profileURL = QLatin1String("https://www.facebook.com");
        uploadPerm = true;
    }

    QString   id;

    QString   name;
    QString   profileURL;
    bool      uploadPerm;
};

// ---------------------------------------------------------------

enum FbPrivacy
{
    FB_ME = 0,
    FB_FRIENDS = 1,
    FB_FRIENDS_OF_FRIENDS,
//     FB_NETWORKS, //NETWORK is deprecated in latest version of Graph API
    FB_EVERYONE,
    FB_CUSTOM
};

// ---------------------------------------------------------------

class FbAlbum: public WSAlbum
{
public:

    explicit FbAlbum()
      : WSAlbum()
    {
        privacy = FB_FRIENDS;
    }

    explicit FbAlbum(const WSAlbum& baseAlbum)
      : WSAlbum(baseAlbum)
    {
        privacy = FB_FRIENDS;
    }

    FbPrivacy privacy;
};

// ---------------------------------------------------------------

class FbPhoto
{
public:

    FbPhoto()
    {
    }

    QString id;

    QString caption;
    QString thumbURL;
    QString originalURL;
};

} // namespace DigikamGenericFaceBookPlugin

#endif // DIGIKAM_FB_ITEM_H
