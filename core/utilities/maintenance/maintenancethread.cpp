/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-08-09
 * Description : Thread actions manager for maintenance tools.
 *
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2017-2018 by Mario Frank <mario dot frank at uni minus potsdam dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "maintenancethread.h"

// Qt includes

#include <QQueue>
#include <QMutex>

// Local includes

#include "digikam_debug.h"
#include "metadatatask.h"
#include "thumbstask.h"
#include "fingerprintstask.h"
#include "imagequalitytask.h"
#include "imagequalitycontainer.h"
#include "databasetask.h"
#include "maintenancedata.h"
#include "imagequalityparser.h"

namespace Digikam
{

MaintenanceThread::MaintenanceThread(QObject* const parent)
    : ActionThreadBase(parent),
      data(new MaintenanceData)
{
    setObjectName(QLatin1String("MaintenanceThread"));

    connect(this, SIGNAL(finished()),
            this, SLOT(slotThreadFinished()));
}

MaintenanceThread::~MaintenanceThread()
{
    cancel();
    wait();
    delete data;
}

void MaintenanceThread::setUseMultiCore(const bool b)
{
    if (!b)
    {
        setMaximumNumberOfThreads(1);
    }
    else
    {
        setDefaultMaximumNumberOfThreads();
        //setMaximumNumberOfThreads(qMax( maximumNumberOfThreads() / 2, 1));
    }
}

void MaintenanceThread::syncMetadata(const ItemInfoList& items, MetadataSynchronizer::SyncDirection dir, bool tagsOnly)
{
    ActionJobCollection collection;

    data->setItemInfos(items);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        MetadataTask* const t = new MetadataTask();
        t->setTagsOnly(tagsOnly);
        t->setDirection(dir);
        t->setMaintenanceData(data);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a metadata task for synchronizing metadata";
    }

    appendJobs(collection);
}

void MaintenanceThread::generateThumbs(const QStringList& paths)
{
    ActionJobCollection collection;

    data->setImagePaths(paths);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        ThumbsTask* const t = new ThumbsTask();

        t->setMaintenanceData(data);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a thumbnails task for generating thumbnails";
    }

    appendJobs(collection);
}

void MaintenanceThread::generateFingerprints(const QList<qlonglong>& itemIds, bool rebuildAll)
{
    ActionJobCollection collection;

    data->setImageIds(itemIds);
    data->setRebuildAllFingerprints(rebuildAll);

    for (int i = 1 ; i <= (maximumNumberOfThreads()) ; ++i)
    {
        FingerprintsTask* const t = new FingerprintsTask();

        t->setMaintenanceData(data);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a fingerprints task for generating fingerprints";
    }

    appendJobs(collection);
}

void MaintenanceThread::sortByImageQuality(const QStringList& paths, const ImageQualityContainer& quality)
{
    ActionJobCollection collection;

    data->setImagePaths(paths);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        ImageQualityTask* const t = new ImageQualityTask();
        t->setQuality(quality);
        t->setMaintenanceData(data);

        connect(t, SIGNAL(signalFinished(QImage)),
                this, SIGNAL(signalAdvance(QImage)));

        connect(this, SIGNAL(signalCanceled()),
                t, SLOT(slotCancel()), Qt::QueuedConnection);

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a image quality task for sorting items.";
    }

    appendJobs(collection);
}

void MaintenanceThread::computeDatabaseJunk(bool thumbsDb, bool facesDb, bool similarityDb)
{
    ActionJobCollection collection;

    DatabaseTask* const t = new DatabaseTask();
    t->setMode(DatabaseTask::Mode::ComputeDatabaseJunk);
    t->computeDatabaseJunk(thumbsDb, facesDb, similarityDb);

    connect(t, SIGNAL(signalFinished()),
            this, SIGNAL(signalAdvance()));

    connect(t, SIGNAL(signalAddItemsToProcess(int)),
            this, SIGNAL(signalAddItemsToProcess(int)));

    connect(t, SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)),
            this, SIGNAL(signalData(QList<qlonglong>,QList<int>,QList<Identity>,QList<qlonglong>)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void MaintenanceThread::cleanCoreDb(const QList<qlonglong>& imageIds)
{
    ActionJobCollection collection;

    data->setImageIds(imageIds);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        DatabaseTask* const t = new DatabaseTask();

        t->setMaintenanceData(data);
        t->setMode(DatabaseTask::Mode::CleanCoreDb);

        connect(t, SIGNAL(signalFinished()),
                this, SIGNAL(signalAdvance()));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a database task for removing stale items.";
    }

    appendJobs(collection);
}

void MaintenanceThread::cleanThumbsDb(const QList<int>& thumbnailIds)
{
    ActionJobCollection collection;

    data->setThumbnailIds(thumbnailIds);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        DatabaseTask* const t = new DatabaseTask();

        t->setMaintenanceData(data);
        t->setMode(DatabaseTask::Mode::CleanThumbsDb);

        connect(t, SIGNAL(signalFinished()),
                this, SIGNAL(signalAdvance()));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a database task for removing stale thumbnails.";
    }

    appendJobs(collection);
}

void MaintenanceThread::cleanFacesDb(const QList<Identity>& staleIdentities)
{
    ActionJobCollection collection;

    data->setIdentities(staleIdentities);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        DatabaseTask* const t = new DatabaseTask();

        t->setMaintenanceData(data);
        t->setMode(DatabaseTask::Mode::CleanRecognitionDb);

        connect(t, SIGNAL(signalFinished()),
                this, SIGNAL(signalAdvance()));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a database task for removing stale identities.";
    }

    appendJobs(collection);
}

void MaintenanceThread::cleanSimilarityDb(const QList<qlonglong>& imageIds)
{
    ActionJobCollection collection;

    data->setSimilarityImageIds(imageIds);

    for (int i = 1 ; i <= maximumNumberOfThreads() ; ++i)
    {
        DatabaseTask* const t = new DatabaseTask();

        t->setMaintenanceData(data);
        t->setMode(DatabaseTask::Mode::CleanSimilarityDb);

        connect(t, SIGNAL(signalFinished()),
                this, SIGNAL(signalAdvance()));

        collection.insert(t, 0);

        qCDebug(DIGIKAM_GENERAL_LOG) << "Creating a database task for removing stale image id entries from similarity db.";
    }

    appendJobs(collection);
}

void MaintenanceThread::shrinkDatabases()
{
    ActionJobCollection collection;

    DatabaseTask* const t = new DatabaseTask();
    t->setMode(DatabaseTask::Mode::ShrinkDatabases);

    connect(t, SIGNAL(signalStarted()),
            this, SIGNAL(signalStarted()));

    connect(t, SIGNAL(signalFinished(bool,bool)),
            this, SIGNAL(signalFinished(bool,bool)));

    collection.insert(t, 0);

    appendJobs(collection);
}

void MaintenanceThread::cancel()
{
    if (isRunning())
    {
        Q_EMIT signalCanceled();
    }

    ActionThreadBase::cancel();
}

void MaintenanceThread::slotThreadFinished()
{
    if (isEmpty())
    {
        ImageQualityParser::unloadDLModel();
        qCDebug(DIGIKAM_GENERAL_LOG) << "List of Pending Jobs is empty";
        Q_EMIT signalCompleted();
    }
}

/*
int MaintenanceThread::getChunkSize(int elementCount)
{
    int chunkSize = elementCount;

    if (maximumNumberOfThreads() > 1)
    {
        chunkSize = elementCount / (maximumNumberOfThreads() - 1);
    }

    // add a factor to the chunk size (do it dynamically depending on free mem?)
    chunkSize = chunkSize * 0.5;

    return chunkSize;
}
*/

} // namespace Digikam
