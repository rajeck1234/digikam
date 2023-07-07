/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-03-22
 * Description : Building complex database SQL queries from search descriptions
 *               Internal containers.
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ITEM_QUERY_BUILDER_P_H
#define DIGIKAM_ITEM_QUERY_BUILDER_P_H

#include "itemquerybuilder.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QFile>
#include <QDir>
#include <QMap>
#include <QRectF>
#include <QUrl>
#include <QLocale>
#include <QUrlQuery>

// Local includes

#include "metaengine.h"
#include "digikam_debug.h"
#include "coredbaccess.h"
#include "coredb.h"
#include "coredbbackend.h"
#include "fieldquerybuilder.h"

namespace Digikam
{

class Q_DECL_HIDDEN RuleTypeForConversion
{
public:

    RuleTypeForConversion();

    QString             key;
    SearchXml::Relation op;
    QString             val;
};

// -------------------------------------------------------------------------

enum SKey
{
    ALBUM = 0,
    ALBUMNAME,
    ALBUMCAPTION,
    ALBUMCOLLECTION,
    TAG,
    TAGNAME,
    IMAGENAME,
    IMAGECAPTION,
    IMAGEDATE,
    KEYWORD,
    RATING
};

enum SOperator
{
    EQ = 0,
    NE,
    LT,
    GT,
    LIKE,
    NLIKE,
    LTE,
    GTE
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN RuleType
{
public:

    SKey      key;
    SOperator op;
    QString   val;
};

// -------------------------------------------------------------------------

class Q_DECL_HIDDEN SubQueryBuilder
{
public:

    QString build(enum SKey key,
                  enum SOperator op,
                  const QString& passedVal,
                  QList<QVariant>* boundValues) const;
};

} // namespace Digikam

#endif // DIGIKAM_ITEM_QUERY_BUILDER_P_H
