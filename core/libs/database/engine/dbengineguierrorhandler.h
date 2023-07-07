/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-27
 * Description : Database engine gui error handler
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_ENGINE_GUI_ERROR_HANDLER_H
#define DIGIKAM_DB_ENGINE_GUI_ERROR_HANDLER_H

// Qt includes

#include <QThread>

// Local includes

#include "dbenginebackend.h"
#include "dbengineerrorhandler.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DbEngineConnectionChecker : public QThread
{
    Q_OBJECT

public:

    explicit DbEngineConnectionChecker(const DbEngineParameters& parameters);
    ~DbEngineConnectionChecker() override;

    bool checkSuccessful() const;

public Q_SLOTS:

    void stopChecking();

protected:

    void run() override;

Q_SIGNALS:

    void failedAttempt();
    void done();

private:

    // Disable
    DbEngineConnectionChecker(QObject*) = delete;

    class Private;
    Private* const d;
};

// --------------------------------------------------------------

class DIGIKAM_EXPORT DbEngineGuiErrorHandler : public DbEngineErrorHandler
{
    Q_OBJECT

public:

    explicit DbEngineGuiErrorHandler(const DbEngineParameters& parameters);
    ~DbEngineGuiErrorHandler() override;

    bool checkDatabaseConnection();

public Q_SLOTS:

    void connectionError(DbEngineErrorAnswer* answer, const QSqlError& error, const QString& query) override;
    void consultUserForError(DbEngineErrorAnswer* answer, const QSqlError& error, const QString& query) override;

private Q_SLOTS:

    void showProgressDialog();

private:

    // Disabled
    DbEngineGuiErrorHandler(QObject*) = delete;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DB_ENGINE_GUI_ERROR_HANDLER_H
