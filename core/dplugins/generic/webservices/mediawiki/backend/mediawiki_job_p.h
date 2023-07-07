/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MEDIAWIKI_JOB_P_H
#define DIGIKAM_MEDIAWIKI_JOB_P_H

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_iface.h"

namespace MediaWiki
{

class Q_DECL_HIDDEN JobPrivate
{
public:

    explicit JobPrivate(Iface& MediaWiki)
        : MediaWiki(MediaWiki),
          manager  (MediaWiki.manager()),
          reply    (nullptr)
    {
    }

    Iface&                       MediaWiki;
    QNetworkAccessManager* const manager;
    QNetworkReply*               reply;
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_JOB_P_H
