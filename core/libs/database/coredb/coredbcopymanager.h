/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : Core database copy manager for migration operations.
 *
 * SPDX-FileCopyrightText: 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * SPDX-FileCopyrightText: 2010-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CORE_DB_COPY_MANAGER_H
#define DIGIKAM_CORE_DB_COPY_MANAGER_H

// Qt includes

#include <QObject>

// Local includes

#include "digikam_export.h"
#include "coredbbackend.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT CoreDbCopyManager : public QObject
{
    Q_OBJECT

public:

    enum FinishStates
    {
        success,
        failed,
        canceled
    };

public:

    CoreDbCopyManager();
    ~CoreDbCopyManager() override;

    void copyDatabases(const DbEngineParameters& fromDBParameters,
                       const DbEngineParameters& toDBParameters);

Q_SIGNALS:

    void stepStarted(const QString& stepName);
    void smallStepStarted(int currValue, int maxValue);
    void finished(int finishState, const QString& errorMsg);

public Q_SLOTS:

    void stopProcessing();

private:

    // Disabled
    explicit CoreDbCopyManager(QObject*) = delete;

    bool copyTable(CoreDbBackend& fromDBbackend,
                   const QString& fromActionName,
                   CoreDbBackend& toDBbackend,
                   const QString& toActionName);

    void handleClosing(bool isstopThread,
                       CoreDbBackend& fromDBbackend,
                       CoreDbBackend& toDBbackend);

private:

    volatile bool m_isStopProcessing;
};

} // namespace Digikam

#endif // DIGIKAM_CORE_DB_COPY_MANAGER_H
