/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-12-17
 * Description : workflow manager.
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BQM_WORKFLOW_MANAGER_H
#define DIGIKAM_BQM_WORKFLOW_MANAGER_H

// Qt includes

#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

// Local includes

#include "queuesettings.h"
#include "batchtoolutils.h"

namespace Digikam
{

/**
 * This container group all queue common settings plus all assigned batch tools.
 */
class Workflow
{
public:

    explicit Workflow() {};
    ~Workflow()         {};

public:

    QString       title;
    QString       desc;

    QueueSettings qSettings;
    BatchSetList  aTools;
};

// -----------------------------------------------------------------------------------------------------------

class WorkflowManager : public QObject
{
    Q_OBJECT

public:

    static WorkflowManager* instance();

public:

    /**
     * Load all Workflow from XML settings file. Fill 'failed' list with incompatible Workflow
     * title/description not loaded.
     */
    bool load(QStringList& failed);

    /**
     * Save all Workflow to  XML settings file.
     */
    bool save();
    void clear();

    void insert(const Workflow& q);
    void remove(const Workflow& q);

    Workflow findByTitle(const QString& title) const;
    QList<Workflow> queueSettingsList()        const;

Q_SIGNALS:

    void signalQueueSettingsAdded(const QString&);
    void signalQueueSettingsRemoved(const QString&);

private:

    void insertPrivate(const Workflow& q);
    void removePrivate(const Workflow& q);

private:

    // Disable
    WorkflowManager();
    WorkflowManager(QObject*) = delete;
    ~WorkflowManager()                                override;

private:

    friend class WorkflowManagerCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_BQM_WORKFLOW_MANAGER_H
