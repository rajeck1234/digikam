/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-12-13
 * Description : test cases for tags tree manipulation in database
 *
 * SPDX-FileCopyrightText: 2015-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "databasetags_utest.h"

// Qt includes

#include <QDir>
#include <QTest>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "albummodel.h"
#include "albumthumbnailloader.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "loadingcacheinterface.h"
#include "scancontroller.h"
#include "thumbnailloadthread.h"
#include "databaseserverstarter.h"
#include "dtestdatadir.h"

using namespace Digikam;

QTEST_MAIN(DatabaseTagsTest)

DatabaseTagsTest::DatabaseTagsTest(QObject* const parent)
    : QObject                (parent),
      palbumRoot0            (nullptr),
      palbumRoot1            (nullptr),
      palbumRoot2            (nullptr),
      palbumChild0Root0      (nullptr),
      palbumChild1Root0      (nullptr),
      palbumChild2Root0      (nullptr),
      palbumChild0Root1      (nullptr),
      rootTag                (nullptr),
      talbumRoot0            (nullptr),
      talbumRoot1            (nullptr),
      talbumChild0Root0      (nullptr),
      talbumChild1Root0      (nullptr),
      talbumChild0Child1Root0(nullptr),
      talbumChild0Root1      (nullptr)
{
        filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/database"))
                       .root().path() + QLatin1Char('/');
        qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << filesPath;
}

DatabaseTagsTest::~DatabaseTagsTest()
{
}

void DatabaseTagsTest::initTestCase()
{
    tempSuffix = QLatin1String("databasetagstest-") + QTime::currentTime().toString();
    dbPath     = QDir::temp().absolutePath() + QLatin1Char('/') + tempSuffix;

    if (QDir::temp().exists(tempSuffix))
    {
        QString msg = QLatin1String("Error creating temp path") + dbPath;
        QVERIFY2(false, msg.toLatin1().constData());
    }

    QDir::temp().mkdir(tempSuffix);

    qCDebug(DIGIKAM_TESTS_LOG) << "Using database path for test: " << dbPath;

    // use a testing database

    AlbumManager::instance();

    AlbumManager::checkDatabaseDirsAfterFirstRun(QDir::temp().absoluteFilePath(
                                                 tempSuffix), QDir::temp().absoluteFilePath(tempSuffix));

    DbEngineParameters params = DbEngineParameters::defaultParameters(QLatin1String("QMYSQL"));
    params.setInternalServerPath(dbPath);
    DatabaseServerStarter::instance()->startServerManagerProcess(params);

    bool dbChangeGood = AlbumManager::instance()->setDatabase(params, false,
                        QDir::temp().absoluteFilePath(tempSuffix));
    QVERIFY2(dbChangeGood, "Could not set temp album db");

    QList<CollectionLocation> locs = CollectionManager::instance()->allAvailableLocations();
    QVERIFY2(locs.size(), "Failed to auto-create one collection in setDatabase");

    ScanController::instance()->completeCollectionScan();
    AlbumManager::instance()->startScan();

    AlbumList all = AlbumManager::instance()->allPAlbums();
    qCDebug(DIGIKAM_TESTS_LOG) << "PAlbum registered : " << all.size();

    Q_FOREACH (Album* const a, all)
    {
        if (a)
        {
            qCDebug(DIGIKAM_TESTS_LOG) << " ==> Id : " << a->id()
                                       << " , is root : " << a->isRoot()
                                       << " , title : " << a->title();
        }
    }

    QVERIFY2( all.size() == 3, "Failed to scan empty directory. We must have one root album, one album, and one trash.");
}

void DatabaseTagsTest::cleanupTestCase()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::cleanupTestCase()";

    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();
    ThumbnailLoadThread::cleanUp();
    AlbumThumbnailLoader::instance()->cleanUp();
    LoadingCacheInterface::cleanUp();

    QDir dir(dbPath);
    dir.removeRecursively();

    qCDebug(DIGIKAM_TESTS_LOG) << "deleted test folder " << dbPath;
}

// Qt test doesn't use exceptions, so using assertion macros in methods called
// from a test slot doesn't stop the test method and may result in inconsistent
// data or segfaults. Therefore use macros for these functions.

#define safeCreatePAlbum(parent, name, result) \
{ \
    QString error; \
    result = AlbumManager::instance()->createPAlbum(parent, name, name, \
                    QDate::currentDate(), albumCategory, error); \
    QVERIFY2(result, QString::fromUtf8("Error creating PAlbum for test: %1").arg(error).toLatin1().constData()); \
}

#define safeCreateTAlbum(parent, name, result) \
{ \
    QString error; \
    result = AlbumManager::instance()->createTAlbum(parent, name, QLatin1String(""), error); \
    QVERIFY2(result, QString::fromUtf8("Error creating TAlbum for test: %1").arg(error).toLatin1().constData()); \
}

void DatabaseTagsTest::init()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::init()";
/*
    palbumCountHash.clear();

    // create a model to check that model work is done correctly while scanning
    addedIds.clear();
    startModel = new AlbumModel;
    startModel->setShowCount(true);

    connect(startModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(slotStartModelRowsInserted(QModelIndex,int,int)));

    connect(startModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(slotStartModelDataChanged(QModelIndex,QModelIndex)));

    qCDebug(DIGIKAM_TESTS_LOG) << "Created startModel" << startModel;

    // ensure that this model is empty in the beginning except for the root
    // album and the collection that include trash
    QCOMPARE(startModel->rowCount(), 1);
    QModelIndex rootIndex = startModel->index(0, 0);
    QCOMPARE(startModel->rowCount(rootIndex), 1);
    QModelIndex collectionIndex = startModel->index(0, 0, rootIndex);
    QCOMPARE(startModel->rowCount(collectionIndex), 1);

    // insert some test data

    // physical albums

    // create two of them by creating directories and scanning
    QDir dir(dbPath);
    dir.mkdir(QLatin1String("root0"));
    dir.mkdir(QLatin1String("root1"));

    ScanController::instance()->completeCollectionScan();
    AlbumManager::instance()->refresh();

    QCOMPARE(AlbumManager::instance()->allPAlbums().size(), 5);

    QString error;
    palbumRoot0 = AlbumManager::instance()->findPAlbum(QUrl::fromLocalFile(dbPath + QLatin1String("/root0")));
    QVERIFY2(palbumRoot0, "Error having PAlbum root0 in AlbumManager");
    palbumRoot1 = AlbumManager::instance()->findPAlbum(QUrl::fromLocalFile(dbPath + QLatin1String("/root1")));
    QVERIFY2(palbumRoot1, "Error having PAlbum root1 in AlbumManager");

    // Create some more through AlbumManager
    palbumRoot2 = AlbumManager::instance()->createPAlbum(dbPath, QLatin1String("root2"),
                  QLatin1String("root album 2"), QDate::currentDate(), albumCategory, error);
    QVERIFY2(palbumRoot2, QString::fromUtf8("Error creating PAlbum for test: %1").arg(error).toLatin1().constData());

    safeCreatePAlbum(palbumRoot0, QLatin1String("root0child0"), palbumChild0Root0);
    safeCreatePAlbum(palbumRoot0, QLatin1String("root0child1"), palbumChild1Root0);
    const QString sameName = QLatin1String("sameName Album");
    safeCreatePAlbum(palbumRoot0, sameName, palbumChild2Root0);

    safeCreatePAlbum(palbumRoot1, sameName, palbumChild0Root1);

    qCDebug(DIGIKAM_TESTS_LOG) << "AlbumManager now knows these PAlbums:";

    Q_FOREACH (Album* const a, AlbumManager::instance()->allPAlbums())
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "\t" << a->title();
    }

    // tags

    rootTag = AlbumManager::instance()->findTAlbum(0);
    QVERIFY(rootTag);

    safeCreateTAlbum(rootTag, QLatin1String("root0"), talbumRoot0);
    safeCreateTAlbum(rootTag, QLatin1String("root1"), talbumRoot1);

    safeCreateTAlbum(talbumRoot0, QLatin1String("child0 root 0"), talbumChild0Root0);
    safeCreateTAlbum(talbumRoot0, QLatin1String("child1 root 0"), talbumChild1Root0);

    safeCreateTAlbum(talbumChild1Root0, sameName, talbumChild0Child1Root0);

    safeCreateTAlbum(talbumRoot1, sameName, talbumChild0Root1);

    qCDebug(DIGIKAM_TESTS_LOG) << "created tags";

    // add some images for having date albums

    QDir imageDir(filesPath);
    imageDir.setNameFilters(QStringList() << QLatin1String("*.jpg"));
    QStringList imageFiles = imageDir.entryList();

    qCDebug(DIGIKAM_TESTS_LOG) << "copying images " << imageFiles << " to "
             << palbumChild0Root0->fileUrl();

    Q_FOREACH (const QString& imageFile, imageFiles)
    {
        QString src = filesPath + QLatin1Char('/') + imageFile;
        QString dst = palbumChild0Root0->fileUrl().toLocalFile() + QLatin1Char('/') + imageFile;
        bool copied = QFile::copy(src, dst);
        QVERIFY2(copied, "Test images must be copied");
    }

    ScanController::instance()->completeCollectionScan();

    if (AlbumManager::instance()->allDAlbums().count() <= 1)
    {
        ensureItemCounts();
    }

    qCDebug(DIGIKAM_TESTS_LOG) << "date albums: " << AlbumManager::instance()->allDAlbums();

    // root + 2 years + 2 and 3 months per year + (1997 as test year for date ordering with 12 months) = 21
    QCOMPARE(AlbumManager::instance()->allDAlbums().size(), 21);
    // ensure that there is a root date album
    DAlbum* const rootFromAlbumManager = AlbumManager::instance()->findDAlbum(0);
    QVERIFY(rootFromAlbumManager);
    DAlbum* rootFromList               = 0;

    Q_FOREACH (Album* const album, AlbumManager::instance()->allDAlbums())
    {
        DAlbum* const dAlbum = dynamic_cast<DAlbum*> (album);
        QVERIFY(dAlbum);

        if (dAlbum->isRoot())
        {
            rootFromList = dAlbum;
        }
    }

    QVERIFY(rootFromList);
    QVERIFY(rootFromList == rootFromAlbumManager);
*/
}

void DatabaseTagsTest::cleanup()
{
/*
    if (startModel)
    {
        disconnect(startModel);
    }

    delete startModel;
    addedIds.clear();

    // remove all test data

    AlbumManager::instance()->refresh();

    // remove all palbums' directories
    deletePAlbum(palbumRoot0);
    deletePAlbum(palbumRoot1);
    deletePAlbum(palbumRoot2);

    // take over changes to database
    ScanController::instance()->completeCollectionScan();

    // reread from database
    AlbumManager::instance()->refresh();

    // root + one collection
    QCOMPARE(AlbumManager::instance()->allPAlbums().size(), 2);

    // remove all tags

    QString error;
    bool removed = AlbumManager::instance()->deleteTAlbum(talbumRoot0, error);
    QVERIFY2(removed, QString::fromUtf8("Error removing a tag: %1").arg(error).toLatin1().constData());
    removed      = AlbumManager::instance()->deleteTAlbum(talbumRoot1, error);
    QVERIFY2(removed, QString::fromUtf8("Error removing a tag: %1").arg(error).toLatin1().constData());

    QCOMPARE(AlbumManager::instance()->allTAlbums().size(), 1);
*/
}

/*
void DatabaseTagsTest::testStartAlbumModel()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testStartAlbumModel()";

    // verify that the start album model got all these changes

    // one root
    QCOMPARE(startModel->rowCount(), 1);
    // one collection
    QModelIndex rootIndex = startModel->index(0, 0);
    QCOMPARE(startModel->rowCount(rootIndex), 1);
    // two albums in the collection
    QModelIndex collectionIndex = startModel->index(0, 0, rootIndex);
    QCOMPARE(startModel->rowCount(collectionIndex), 3);
    // this is should be enough for now

    // We must have received an added notation for everything except album root
    // and collection
    QCOMPARE(addedIds.size(), 7);
}

void DatabaseTagsTest::ensureItemCounts()
{
    // trigger listing job
    QEventLoop dAlbumLoop;

    connect(AlbumManager::instance(), SIGNAL(signalAllDAlbumsLoaded()),
            &dAlbumLoop, SLOT(quit()));

    AlbumManager::instance()->prepareItemCounts();
    qCDebug(DIGIKAM_TESTS_LOG) << "Waiting for AlbumManager and the IOSlave to create DAlbums...";
    dAlbumLoop.exec();
    qCDebug(DIGIKAM_TESTS_LOG) << "DAlbums were created";

    while (palbumCountHash.size() < 8)
    {
        QEventLoop pAlbumLoop;

        connect(AlbumManager::instance(), SIGNAL(signalPAlbumsDirty(QHash<int,int>)),
                &pAlbumLoop, SLOT(quit()));

        qCDebug(DIGIKAM_TESTS_LOG) << "Waiting for first PAlbum count hash";
        pAlbumLoop.exec();
        qCDebug(DIGIKAM_TESTS_LOG) << "Got new PAlbum count hash";
    }
}

void DatabaseTagsTest::slotStartModelRowsInserted(const QModelIndex& parent, int start, int end)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "called, parent:" << parent << ", start:" << start << ", end:" << end;

    for (int row = start; row <= end; ++row)
    {
        QModelIndex child = startModel->index(row, 0, parent);
        QVERIFY(child.isValid());
        Album* album      = startModel->albumForIndex(child);
        const int id      = child.data(AbstractAlbumModel::AlbumIdRole).toInt();
        QVERIFY(album);
        qCDebug(DIGIKAM_TESTS_LOG) << "added album with id"
                 << id
                 << "and name" << album->title();
        addedIds << id;
    }
}

void DatabaseTagsTest::slotStartModelDataChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    for (int row = topLeft.row(); row <= bottomRight.row(); ++row)
    {
        QModelIndex index = startModel->index(row, topLeft.column(), topLeft.parent());

        if (!index.isValid())
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Illegal index received";
            continue;
        }

        int albumId = index.data(AbstractAlbumModel::AlbumIdRole).toInt();

        if (!addedIds.contains(albumId))
        {
            QString message = QLatin1String("Album id ") + QString::number(albumId) +
                              QLatin1String(" was changed before adding signal was received");
            QFAIL(message.toLatin1().constData());
            qCDebug(DIGIKAM_TESTS_LOG) << message;
        }
    }
}

void DatabaseTagsTest::deletePAlbum(PAlbum* album)
{
    QDir dir(album->folderPath());
    dir.removeRecursively();
}

void DatabaseTagsTest::setLastPAlbumCountHash(const QHash<int, int> &hash)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Receiving new count hash" << hash;
    palbumCountHash = hash;
}

void DatabaseTagsTest::testPAlbumModel()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testPAlbumModel()";

    AlbumModel* albumModel = new AlbumModel();
    ModelTest* test        = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;

    albumModel = new AlbumModel(AbstractAlbumModel::IgnoreRootAlbum);
    test       = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}

void DatabaseTagsTest::testDisablePAlbumCount()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testDisablePAlbumCount()";

    AlbumModel albumModel;
    albumModel.setCountHash(palbumCountHash);
    albumModel.setShowCount(true);

    QRegExp countRegEx(QLatin1String(".+ \\(\\d+\\)"));
    countRegEx.setMinimal(true);
    QVERIFY(countRegEx.exactMatch(QLatin1String("test (10)")));
    QVERIFY(countRegEx.exactMatch(QLatin1String("te st (10)")));
    QVERIFY(countRegEx.exactMatch(QLatin1String("te st (0)")));
    QVERIFY(!countRegEx.exactMatch(QLatin1String("te st ()")));
    QVERIFY(!countRegEx.exactMatch(QLatin1String("te st")));
    QVERIFY(!countRegEx.exactMatch(QLatin1String("te st (10) bla")));

    // ensure that all albums except the root album have a count attached
    QModelIndex rootIndex = albumModel.index(0, 0, QModelIndex());
    QString rootTitle     = albumModel.data(rootIndex, Qt::DisplayRole).toString();
    QVERIFY(!countRegEx.exactMatch(rootTitle));

    for (int collectionRow = 0; collectionRow < albumModel.rowCount(rootIndex); ++collectionRow)
    {
        QModelIndex collectionIndex = albumModel.index(collectionRow, 0, rootIndex);
        QString collectionTitle = albumModel.data(collectionIndex, Qt::DisplayRole).toString();
        QVERIFY2(countRegEx.exactMatch(collectionTitle), QString::fromUtf8("%1 matching error").arg(collectionTitle).toLatin1().constData());

        for (int albumRow = 0; albumRow < albumModel.rowCount(collectionIndex); ++albumRow)
        {
            QModelIndex albumIndex = albumModel.index(albumRow, 0, collectionIndex);
            QString albumTitle     = albumModel.data(albumIndex, Qt::DisplayRole).toString();
            QVERIFY2(countRegEx.exactMatch(albumTitle), QString::fromUtf8("%1 matching error").arg(albumTitle).toLatin1().constData());
        }

    }

    // now disable showing the count
    albumModel.setShowCount(false);

    // ensure that no album has a count attached
    rootTitle = albumModel.data(rootIndex, Qt::DisplayRole).toString();
    QVERIFY(!countRegEx.exactMatch(rootTitle));

    for (int collectionRow = 0; collectionRow < albumModel.rowCount(rootIndex); ++collectionRow)
    {
        QModelIndex collectionIndex = albumModel.index(collectionRow, 0, rootIndex);
        QString collectionTitle     = albumModel.data(collectionIndex, Qt::DisplayRole).toString();
        QVERIFY2(!countRegEx.exactMatch(collectionTitle), QString::fromUtf8("%1 matching error").arg(collectionTitle).toLatin1().constData());

        for (int albumRow = 0; albumRow < albumModel.rowCount(collectionIndex); ++albumRow)
        {
            QModelIndex albumIndex = albumModel.index(albumRow, 0, collectionIndex);
            QString albumTitle     = albumModel.data(albumIndex, Qt::DisplayRole).toString();
            QVERIFY2(!countRegEx.exactMatch(albumTitle), QString::fromUtf8("%1 matching error").arg(albumTitle).toLatin1().constData());
        }
    }
}

void DatabaseTagsTest::testDAlbumModel()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testDAlbumModel()";

    DateAlbumModel* const albumModel = new DateAlbumModel();
    ensureItemCounts();
    ModelTest* const test            = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}

void DatabaseTagsTest::testDAlbumContainsAlbums()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testDAlbumContainsAlbums()";

    DateAlbumModel* const albumModel = new DateAlbumModel();
    ensureItemCounts();

    QVERIFY(albumModel->rootAlbum());

    Q_FOREACH (Album* const album, AlbumManager::instance()->allDAlbums())
    {
        DAlbum* const dAlbum = dynamic_cast<DAlbum*> (album);
        QVERIFY(dAlbum);

        qCDebug(DIGIKAM_TESTS_LOG) << "checking album for date " << dAlbum->date() << ", range = " << dAlbum->range();

        QModelIndex index = albumModel->indexForAlbum(dAlbum);

        if (!dAlbum->isRoot())
        {
            QVERIFY(index.isValid());
        }

        if (dAlbum->isRoot())
        {
            // root album
            QVERIFY(dAlbum->isRoot());
            QCOMPARE(albumModel->rowCount(index), 3);
            QCOMPARE(index, albumModel->rootAlbumIndex());
        }
        else if (dAlbum->range() == DAlbum::Year && dAlbum->date().year() == 2007)
        {
            QCOMPARE(albumModel->rowCount(index), 2);
        }
        else if (dAlbum->range() == DAlbum::Year && dAlbum->date().year() == 2009)
        {
            QCOMPARE(albumModel->rowCount(index), 3);
        }
        else if (dAlbum->range() == DAlbum::Month && dAlbum->date().year() == 2007 && dAlbum->date().month() == 3)
        {
            QCOMPARE(albumModel->rowCount(index), 0);
        }
        else if (dAlbum->range() == DAlbum::Month && dAlbum->date().year() == 2007 && dAlbum->date().month() == 4)
        {
            QCOMPARE(albumModel->rowCount(index), 0);
        }
        else if (dAlbum->range() == DAlbum::Month && dAlbum->date().year() == 2009 && dAlbum->date().month() == 3)
        {
            QCOMPARE(albumModel->rowCount(index), 0);
        }
        else if (dAlbum->range() == DAlbum::Month && dAlbum->date().year() == 2009 && dAlbum->date().month() == 4)
        {
            QCOMPARE(albumModel->rowCount(index), 0);
        }
        else if (dAlbum->range() == DAlbum::Month && dAlbum->date().year() == 2009 && dAlbum->date().month() == 5)
        {
            QCOMPARE(albumModel->rowCount(index), 0);
        }
        else if (dAlbum->date().year() == 1997)
        {
            // Ignore these albums for order testing
        }
        else
        {
            qCDebug(DIGIKAM_TESTS_LOG) << "Unexpected album: " << dAlbum->title();
            QFAIL("Unexpected album returned from model");
        }
    }

    delete albumModel;
}

void DatabaseTagsTest::testDAlbumSorting()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testDAlbumSorting()";

    DateAlbumModel dateAlbumModel;
    AlbumFilterModel albumModel;
    albumModel.setSourceAlbumModel(&dateAlbumModel);

    // first check ascending order
    albumModel.sort(0, Qt::AscendingOrder);
    int previousYear = 0;

    for (int yearRow = 0; yearRow < albumModel.rowCount(); ++yearRow)
    {
        QModelIndex yearIndex   = albumModel.index(yearRow, 0);
        DAlbum* const yearAlbum = dynamic_cast<DAlbum*> (albumModel.albumForIndex(yearIndex));
        QVERIFY(yearAlbum);

        QVERIFY(yearAlbum->date().year() > previousYear);
        previousYear = yearAlbum->date().year();

        int previousMonth = 0;

        for (int monthRow = 0; monthRow < albumModel.rowCount(yearIndex); ++monthRow)
        {
            QModelIndex monthIndex   = albumModel.index(monthRow, 0, yearIndex);
            DAlbum* const monthAlbum = dynamic_cast<DAlbum*> (albumModel.albumForIndex(monthIndex));
            QVERIFY(monthAlbum);

            QVERIFY(monthAlbum->date().month() > previousMonth);
            previousMonth = monthAlbum->date().month();
        }
    }

    // then check descending order
    albumModel.sort(0, Qt::DescendingOrder);
    previousYear = 1000000;

    for (int yearRow = 0; yearRow < albumModel.rowCount(); ++yearRow)
    {
        QModelIndex yearIndex   = albumModel.index(yearRow, 0);
        DAlbum* const yearAlbum = dynamic_cast<DAlbum*> (albumModel.albumForIndex(yearIndex));
        QVERIFY(yearAlbum);

        QVERIFY(yearAlbum->date().year() < previousYear);
        previousYear          = yearAlbum->date().year();

        int previousMonth     = 13;

        for (int monthRow = 0; monthRow < albumModel.rowCount(yearIndex); ++monthRow)
        {
            QModelIndex monthIndex   = albumModel.index(monthRow, 0, yearIndex);
            DAlbum* const monthAlbum = dynamic_cast<DAlbum*> (albumModel.albumForIndex(monthIndex));
            QVERIFY(monthAlbum);

            QVERIFY(monthAlbum->date().month() < previousMonth);
            previousMonth          = monthAlbum->date().month();
        }
    }
}

void DatabaseTagsTest::testDAlbumCount()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testDAlbumCount()";

    DateAlbumModel* const albumModel = new DateAlbumModel();
    albumModel->setShowCount(true);
    ensureItemCounts();

    qCDebug(DIGIKAM_TESTS_LOG) << "iterating over root indices";

    // check year albums
    for (int yearRow = 0; yearRow < albumModel->rowCount(albumModel->rootAlbumIndex()); ++yearRow)
    {
        QModelIndex yearIndex    = albumModel->index(yearRow, 0);
        DAlbum* const yearDAlbum = albumModel->albumForIndex(yearIndex);
        QVERIFY(yearDAlbum);

        QVERIFY(yearDAlbum->range() == DAlbum::Year);

        if (yearDAlbum->date().year() == 2007)
        {
            const int imagesInYear = 7;
            albumModel->includeChildrenCount(yearIndex);
            QCOMPARE(albumModel->albumCount(yearDAlbum), imagesInYear);
            albumModel->excludeChildrenCount(yearIndex);
            QCOMPARE(albumModel->albumCount(yearDAlbum), 0);
            albumModel->includeChildrenCount(yearIndex);
            QCOMPARE(albumModel->albumCount(yearDAlbum), imagesInYear);

            for (int monthRow = 0; monthRow < albumModel->rowCount(yearIndex); ++monthRow)
            {
                QModelIndex monthIndex = albumModel->index(monthRow, 0, yearIndex);
                DAlbum* monthDAlbum    = albumModel->albumForIndex(monthIndex);
                QVERIFY(monthDAlbum);

                QVERIFY(monthDAlbum->range() == DAlbum::Month);
                QVERIFY(monthDAlbum->date().year() == 2007);

                if (monthDAlbum->date().month() == 3)
                {
                    const int imagesInMonth = 3;
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->excludeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                }
                else if (monthDAlbum->date().month() == 4)
                {
                    const int imagesInMonth = 4;
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->excludeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                }
                else
                {
                    QFAIL("unexpected month album in 2007");
                }
            }
        }
        else if (yearDAlbum->date().year() == 2009)
        {
            const int imagesInYear = 5;
            albumModel->includeChildrenCount(yearIndex);
            QCOMPARE(albumModel->albumCount(yearDAlbum), imagesInYear);
            albumModel->excludeChildrenCount(yearIndex);
            QCOMPARE(albumModel->albumCount(yearDAlbum), 0);
            albumModel->includeChildrenCount(yearIndex);
            QCOMPARE(albumModel->albumCount(yearDAlbum), imagesInYear);

            for (int monthRow = 0; monthRow < albumModel->rowCount(yearIndex); ++monthRow)
            {
                QModelIndex monthIndex = albumModel->index(monthRow, 0, yearIndex);
                DAlbum* monthDAlbum    = albumModel->albumForIndex(monthIndex);
                QVERIFY(monthDAlbum);

                QVERIFY(monthDAlbum->range() == DAlbum::Month);
                QVERIFY(monthDAlbum->date().year() == 2009);

                if (monthDAlbum->date().month() == 3)
                {
                    const int imagesInMonth = 2;
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->excludeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                }
                else if (monthDAlbum->date().month() == 4)
                {
                    const int imagesInMonth = 2;
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->excludeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                }
                else if (monthDAlbum->date().month() == 5)
                {
                    const int imagesInMonth = 1;
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->excludeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                    albumModel->includeChildrenCount(monthIndex);
                    QCOMPARE(albumModel->albumCount(monthDAlbum), imagesInMonth);
                }
                else
                {
                    QFAIL("unexpected month album in 2009");
                }
            }
        }
        else if (yearDAlbum->date().year() == 1997)
        {
            // Nothing to do here, ignore the albums for ordering tests
        }
        else
        {
            QFAIL("Received unexpected album from model");
        }
    }

    delete albumModel;
}

void DatabaseTagsTest::testTAlbumModel()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testTAlbumModel()";

    TagModel* albumModel = new TagModel();
    ModelTest* test      = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;

    albumModel = new TagModel(AbstractAlbumModel::IgnoreRootAlbum);
    test       = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}

void DatabaseTagsTest::testSAlbumModel()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Start DatabaseTagsTest::testSAlbumModel()";

    SearchModel* const albumModel = new SearchModel();
    ModelTest* const test         = new ModelTest(albumModel, 0);
    delete test;
    delete albumModel;
}
*/
