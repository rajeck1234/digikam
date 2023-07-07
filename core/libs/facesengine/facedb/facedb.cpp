/* ============================================================
 *
 * This file is a part of digiKam
 *
 * Date        : 02-02-2012
 * Description : Face database interface to train identities.
 *
 * SPDX-FileCopyrightText: 2012-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText:      2019 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "facedb_p.h"

namespace Digikam
{

FaceDb::FaceDb(FaceDbBackend* const db)
    : d(new Private)
{
    d->db = db;
}

FaceDb::~FaceDb()
{
    delete d;
}

BdEngineBackend::QueryState FaceDb::setSetting(const QString& keyword, const QString& value)
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    parameters.insert(QLatin1String(":value"), value);

    return d->db->execDBAction(d->db->getDBAction(QLatin1String("ReplaceFaceSetting")), parameters);
}

QString FaceDb::setting(const QString& keyword) const
{
    QMap<QString, QVariant> parameters;
    parameters.insert(QLatin1String(":keyword"), keyword);
    QList<QVariant> values;

    // TODO Should really check return status here

    BdEngineBackend::QueryState queryStateResult = d->db->execDBAction(d->db->getDBAction(QLatin1String("SelectFaceSetting")), parameters, &values);
    qCDebug(DIGIKAM_FACEDB_LOG) << "FaceDB SelectFaceSetting val ret = " << (BdEngineBackend::QueryStateEnum)queryStateResult;

    if (values.isEmpty())
    {
        return QString();
    }
    else
    {
        return values.first().toString();
    }
}

bool FaceDb::integrityCheck()
{
    QList<QVariant> values;
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("checkRecognitionDbIntegrity")), &values);

    switch (d->db->databaseType())
    {
        case BdEngineBackend::DbType::SQLite:
        {
            // For SQLite the integrity check returns a single row with one string column "ok" on success and multiple rows on error.

            return(
                    (values.size() == 1) &&
                    (values.first().toString().toLower().compare(QLatin1String("ok")) == 0)
                  );
        }

        case BdEngineBackend::DbType::MySQL:
        {
            // For MySQL, for every checked table, the table name, operation (check), message type (status) and the message text (ok on success)
            // are returned. So we check if there are four elements and if yes, whether the fourth element is "ok".
/*
            qCDebug(DIGIKAM_DATABASE_LOG) << "MySQL check returned " << values.size() << " rows";
*/
            if ((values.size() % 4) != 0)
            {
                return false;
            }

            for (QList<QVariant>::iterator it = values.begin() ; it != values.end() ; )
            {
                QString tableName   = (*it).toString();
                ++it;
                QString operation   = (*it).toString();
                ++it;
                QString messageType = (*it).toString();
                ++it;
                QString messageText = (*it).toString();
                ++it;

                if (messageText.toLower().compare(QLatin1String("ok")) != 0)
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Failed integrity check for table " << tableName << ". Reason:" << messageText;
                    return false;
                }
                else
                {
                    qCDebug(DIGIKAM_DATABASE_LOG) << "Passed integrity check for table " << tableName;
                }
            }

            // No error conditions. Db passed the integrity check.

            return true;
        }

        default:
        {
            return false;
        }
    }
}

void FaceDb::vacuum()
{
    d->db->execDBAction(d->db->getDBAction(QString::fromUtf8("vacuumRecognitionDB")));
}

} // namespace Digikam
