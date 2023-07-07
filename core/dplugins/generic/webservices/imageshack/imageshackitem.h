/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-02
 * Description : a tool to export items to ImageShack web service
 *
 * SPDX-FileCopyrightText: 2012      by Dodon Victor <dodonvictor at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2018 by Caulier Gilles <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_IMAGE_SHACK_ITEM_H
#define DIGIKAM_IMAGE_SHACK_ITEM_H

// Qt includes

#include <QString>

namespace DigikamGenericImageShackPlugin
{

class ImageShackGallery
{

public:

    explicit ImageShackGallery()
    {
    }

    QString m_user;
    QString m_server;
    QString m_name;
    QString m_title;
    QString m_url;
};

// -------------------------------------------------------------------

class ImageShackPhoto
{

public:

    explicit ImageShackPhoto()
    {
    }

    QString m_server;
    QString m_name;
    QString m_bucket;
};

} // namespace DigikamGenericImageShackPlugin

#endif // DIGIKAM_IMAGE_SHACK_ITEM_H
