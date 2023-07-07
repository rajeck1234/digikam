/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-27
 * Description : Database engine SQL query
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_ENGINE_SQL_QUERY_H
#define DIGIKAM_DB_ENGINE_SQL_QUERY_H

// Qt includes

#include <QMetaType>
#include <QString>
#include <QSqlQuery>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineSqlQuery : public QSqlQuery
{

public:

    explicit DbEngineSqlQuery(const QSqlQuery& other);
    explicit DbEngineSqlQuery(const QSqlDatabase& db);
    ~DbEngineSqlQuery()                                 = default;

    DbEngineSqlQuery& operator=(const DbEngineSqlQuery& other);

    bool prepare(const QString& query);
    QString lastQuery()                                 const;

private:

    QString m_query;
};

} // namespace Digikam

#endif // DIGIKAM_DB_ENGINE_SQL_QUERY_H
