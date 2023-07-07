/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-02-20
 * Description : Unit tests for TagsCache class
 *
 * SPDX-FileCopyrightText: 2021 by David Haslam, <dch dot code at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "haariface_utest.h"

// C++ includes

#include <set>

// Qt includes

#include <QString>
#include <QDebug>
#include <QSqlDatabase>

// Local includes
#include "digikam_debug.h"
#include "haariface.h"
#include "duplicatesfinder.h"
#include "albumselectors.h"
#include "album.h"
#include "findduplicatesalbumitem.h"

// database related
#include "dtestdatadir.h"
#include "dbengineparameters.h"
#include "albummanager.h"
#include "collectionmanager.h"
#include "collectionlocation.h"
#include "facedbaccess.h"
#include "similaritydbaccess.h"
#include "similaritydb.h"
#include "coredbaccess.h"
#include "thumbsdbaccess.h"
#include "scancontroller.h"

#include <iostream>

using namespace Digikam;

typedef QString ImagePath;

#define ENABLE_TIMEOUT 0

#define PATHFROMFILEINFO(info) \
    QDir(filesPath).relativeFilePath(info.filePath())

#define START_SEARCHING_DUPLICATES \
do {\
    AlbumManager::instance()->clearCurrentAlbums();\
    SimilarityDbAccess().db()->clearImageSimilarity(); \
    /* 50% because largerSmaler.png has only 58% similarity */ \
    DuplicatesFinder* finder = new DuplicatesFinder(searchAlbums, tags, HaarIface::AlbumTagRelation::NoMix, \
                        50, 100,  \
                        HaarIface::DuplicatesSearchRestrictions::None, \
                        refImageSelMethod, referenceAlbums); \
    bool complete = false; \
    connect(finder, &DuplicatesFinder::signalComplete, [&complete]() { \
        complete = true; \
    }); \
    finder->start(); \
    auto startTime = QDateTime::currentMSecsSinceEpoch(); \
    while (!complete) { \
        QTest::qWait(100); \
        if (ENABLE_TIMEOUT)\
            QVERIFY(QDateTime::currentMSecsSinceEpoch() - startTime < 1000); \
    } \
    QTest::qWait(1000); /* Wait until AlbumManager refreshed the salbums */ \
    \
    AlbumList aList = AlbumManager::instance()->allSAlbums(); \
    for (AlbumList::iterator it = aList.begin() ; it != aList.end() ; ++it) { \
        SAlbum* salbum = dynamic_cast<SAlbum*>(*it); \
        if (salbum) \
            salbum->removeExtraData(this); \
    }\
    \
    QTreeWidget w; \
    for (AlbumList::const_iterator it = aList.constBegin() ; it != aList.constEnd() ; ++it) \
    { \
        SAlbum* const salbum = dynamic_cast<SAlbum*>(*it); \
    \
        if (salbum && salbum->isDuplicatesSearch() && !salbum->extraData(this)) \
        { \
            /* Adding item to listView by creating an item and passing listView as parent */  \
            FindDuplicatesAlbumItem* const item = new FindDuplicatesAlbumItem(&w, salbum); \
            salbum->setExtraData(this, item); \
            const auto id = salbum->title().toLongLong(); \
            ItemInfo info(id); \
            const auto path = QDir(filesPath).relativeFilePath(info.filePath()); \
            const QList<ItemInfo> duplicates = item->duplicatedItems(); \
    \
            if (!references.contains(path)) \
                references.insert(path, duplicates); \
        } \
    } \
} while(false);

HaarIfaceTest::HaarIfaceTest(QObject* const parent)
    : QObject  (parent)
{
    filesPath = DTestDataDir::TestData(QString::fromUtf8("core/tests/database/duplicates"))
                   .root().path() + QLatin1Char('/');
    qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << filesPath;
}

void HaarIfaceTest::initTestCase()
{
    auto dir = QDir(filesPath);
    startSqlite(dir);

    // Update collection path, because this is hardcoded

    for (const auto& col: CollectionManager::instance()->allLocations())
    {
        CollectionManager::instance()->removeLocation(col);
    }

    QVERIFY(dir.cd(QStringLiteral("Collection")));
    // The new collection is in the same path as the database, but in the "Collection" subfolder
    const auto collectionPath = dir.absolutePath();
    CollectionManager::instance()->addLocation(QUrl::fromLocalFile(collectionPath), QStringLiteral("Collection"));

    ScanController::instance()->completeCollectionScan();
    ScanController::instance()->allowToScanDeferredFiles();
    AlbumManager::instance()->startScan();
}

void HaarIfaceTest::cleanupTestCase()
{
    stopSql();
}

void HaarIfaceTest::startSqlite(const QDir& dbDir)
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Setup Sqlite Database...";

    if (!QSqlDatabase::isDriverAvailable(DbEngineParameters::SQLiteDatabaseType()))
    {
        QWARN("Qt SQlite plugin is missing.");
        return;
    }

    DbEngineParameters params;
    params.databaseType = DbEngineParameters::SQLiteDatabaseType();
    params.setCoreDatabasePath(dbDir.path() + QLatin1String("/digikam4.db"));
    params.setThumbsDatabasePath(dbDir.path() + QLatin1String("/thumbnails-digikam.db"));
    params.setFaceDatabasePath(dbDir.path() + QLatin1String("/recognition.db"));
    params.setSimilarityDatabasePath(dbDir.path() + QLatin1String("/similarity.db"));
    params.legacyAndDefaultChecks();

    // ------------------------------------------------------------------------------------

    qCDebug(DIGIKAM_TESTS_LOG) << "Initializing SQlite database...";
    QVERIFY2(AlbumManager::instance()->setDatabase(params, false, filesPath, true),
             "Cannot initialize Sqlite database");

    QTest::qWait(3000);
}

void HaarIfaceTest::stopSql()
{
    qCDebug(DIGIKAM_TESTS_LOG) << "Shutting down SQlite database";
    ScanController::instance()->shutDown();
    AlbumManager::instance()->cleanUp();

    qCDebug(DIGIKAM_TESTS_LOG) << "Cleaning Sqlite database";
    CoreDbAccess::cleanUpDatabase();
    ThumbsDbAccess::cleanUpDatabase();
    FaceDbAccess::cleanUpDatabase();
}

// Collection/2020/LargerSmaler.png
// Collection/2021/2.png
// Collection/2022/3.png
// Collection/2023/4.png
// Collection/potentialDuplicates/4.png
// Collection/potentialDuplicates/5.png
// Collection/potentialDuplicates/subfolder/2.png
// Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaler.png

/*!
 * \brief HaarIfaceTest::testOriginal
 * Original behaviour
 */
void HaarIfaceTest::testOriginal()
{
    AlbumList all = AlbumManager::instance()->allPAlbums();

    const auto refImageSelMethod = HaarIface::RefImageSelMethod::OlderOrLarger;
    AlbumList tags; // empty
    AlbumList searchAlbums = all;
    AlbumList referenceAlbums;

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES

    QCOMPARE(references.count(), 3);

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png"))); // This one is larger
        auto duplicates = references.value(QStringLiteral("Collection/2020/LargerSmaller.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"))); // This one is Older
        auto duplicates = references.value(QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/2021/2.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png"))); // exactly same
        auto duplicates = references.value(QStringLiteral("Collection/2023/4.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/4.png"));
    }
}

/*!
 * \brief HaarIfaceTest::testExcludeRefSelectpotentialDuplicates
 * The selected folder shall not be used as reference
 */
void HaarIfaceTest::testExcludeRefSelectpotentialDuplicates()
{
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::ExcludeFolder;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums = all;
    AlbumList referenceAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // potentialDuplicates and subfolders
            referenceAlbums << a;
        }
    }

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES

    QCOMPARE(references.count(), 3);

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2020/LargerSmaller.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2021/2.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2021/2.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2023/4.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/4.png"));
    }
}

/*!
 * \brief HaarIfaceTest::testPreferFolderSelectpotentialDuplicates
 * The selected folder is prefered as reference
 */
void HaarIfaceTest::testPreferFolderSelectpotentialDuplicates()
{
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::PreferFolder;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums = all;
    AlbumList referenceAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // potentialDuplicates and subfolders
            referenceAlbums << a;
        }
    }

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES;

    QCOMPARE(references.count(), 3);

    {
        QVERIFY(references.contains(QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png")));
        auto duplicates = references.value(QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/2020/LargerSmaller.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/potentialDuplicates/subfolder/2.png")));
        auto duplicates = references.value(QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/2021/2.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/potentialDuplicates/4.png")));
        auto duplicates = references.value(QStringLiteral("Collection/potentialDuplicates/4.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/2023/4.png"));
    }
}

void HaarIfaceTest::testPreferNewerCreationDate()
{
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::NewerCreationDate;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums = all;
    AlbumList referenceAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // potentialDuplicates and subfolders
            referenceAlbums << a;
        }
    }

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES;

    QCOMPARE(references.count(), 3);

    // Undefined which one is used, because both have the same time, but at least one of the is available
    QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png")) != QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));

    // Both have same creation date
    QVERIFY(references.contains(QStringLiteral("Collection/2021/2.png")) != QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));

    // Undefined which one is used, because both have the same time, but at least one of the is available
    QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png")) != QStringLiteral("Collection/potentialDuplicates/4.png"));
}

void HaarIfaceTest::testPreferNewerModificationDate()
{
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::NewerModificationDate;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums = all;
    AlbumList referenceAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // potentialDuplicates and subfolders
            referenceAlbums << a;
        }
    }

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES;

    QCOMPARE(references.count(), 3);

    // Undefined which one is used, because both have the same time, but at least one of the is available
    QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png")) != QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2021/2.png"))); // This one is Newer
        auto duplicates = references.value(QStringLiteral("Collection/2021/2.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
    }

    // Undefined which one is used, because both have the same time, but at least one of the is available
    QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png")) != QStringLiteral("Collection/potentialDuplicates/4.png"));
}

void HaarIfaceTest::testPreferFolderWhole()
{
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::PreferFolder;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums = all;
    AlbumList referenceAlbums = all;

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES;

    QCOMPARE(references.count(), 3);

    // Not relevant which one is used, but at least one of the is available
    QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png")) != QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));
    QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png")) != QStringLiteral("Collection/potentialDuplicates/4.png"));
    QVERIFY(references.contains(QStringLiteral("Collection/2021/2.png")) != QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
}

/*!
 * \brief HaarIfaceTest::testReferenceFolderNotSelected
 * The reference folder is not in the search folders.
 * But neverthless the folder shall be checked, otherwise
 * the duplicates get not found. The Duplicatesfinder will merge interally the
 * two album lists
 * The oposite that the folder is in both lists will be tested above, so
 * no extra test is needed.
 */
void HaarIfaceTest::testReferenceFolderNotSelected()
{
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::ExcludeFolder;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (!path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // exclude potential duplicates
            // Collection/2020
            // Collection/2021
            // Collection/2022
            // Collection/2023
            searchAlbums << a;
        }
    }

    AlbumList referenceAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // potentialDuplicates
            referenceAlbums << a;
        }
    }

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES;

    QCOMPARE(references.count(), 3);

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2020/LargerSmaller.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2021/2.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2021/2.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2023/4.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/4.png"));
    }
}

/*!
 * \brief HaarIfaceTest::testReferenceFolderNotSelected
 * Similar test to testReferenceFolderNotSelected(), but with the difference,
 * "Collection/potentialDuplicates/subfolder" is in both, the reference and in the search albums
 */
void HaarIfaceTest::testReferenceFolderPartlySelected() {
    const auto refImageSelMethod = HaarIface::RefImageSelMethod::ExcludeFolder;

    AlbumList all = AlbumManager::instance()->allPAlbums();

    AlbumList tags; // empty
    AlbumList searchAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (!path.startsWith(QStringLiteral("/potentialDuplicates")) || (path.startsWith(QStringLiteral("/potentialDuplicates/subfolder")) &&
                                                                         !path.contains(QStringLiteral("subsubfolder"))))
        {
            // exclude potential duplicates but not the subfolder
            // Collection/2020
            // Collection/2021
            // Collection/2022
            // Collection/2023
            // Collection/potentialDuplicates/subfolder
            searchAlbums << a;
        }
    }

    AlbumList referenceAlbums;

    for (auto* a: all)
    {
        const auto& path = static_cast<PAlbum*>(a)->albumPath();

        if (path.startsWith(QStringLiteral("/potentialDuplicates")))
        {
            // potentialDuplicates
            referenceAlbums << a;
        }
    }

    QHash<ImagePath, QList<ItemInfo>> references;
    START_SEARCHING_DUPLICATES;

    QCOMPARE(references.count(), 3);

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2020/LargerSmaller.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2020/LargerSmaller.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/subsubfolder/LargerSmaller.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2021/2.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2021/2.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/subfolder/2.png"));
    }

    {
        QVERIFY(references.contains(QStringLiteral("Collection/2023/4.png")));
        auto duplicates = references.value(QStringLiteral("Collection/2023/4.png"));
        QCOMPARE(duplicates.count(), 1);
        QCOMPARE(PATHFROMFILEINFO(duplicates.at(0)), QStringLiteral("Collection/potentialDuplicates/4.png"));
    }
}

HaarIfaceTest::~HaarIfaceTest()
{
}


QTEST_MAIN(HaarIfaceTest)
