/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015
 * Description : a test for timestamp trigger for re-reading metadata from image
 *
 * SPDX-FileCopyrightText: 2015 by Kristian Karl <kristian dot hermann dot karl at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "timestampupdate_utest.h"

// Qt includes

#include <QTest>
#include <QFileInfo>
#include <QScopedPointer>

// Local includes

#include "dmetadata.h"
#include "coredb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "collectionscanner.h"
#include "iteminfo.h"
#include "metaenginesettings.h"
#include "dtestdatadir.h"
#include "digikam_debug.h"

const QString originalImageFolder(DTestDataDir::TestData(QString::fromUtf8("core/tests/timestampupdate"))
                                  .root().path() + QLatin1Char('/'));
const QString originalImageFile(DTestDataDir::TestData(QString::fromUtf8("core/tests/timestampupdate"))
                                .root().path() + QLatin1String("/1.jpg"));

QTEST_GUILESS_MAIN(TimeStampUpdateTest)

using namespace Digikam;

QString TimeStampUpdateTest::tempFileName(const QString& purpose) const
{
    return QLatin1String("digikamtests-") + QLatin1String(metaObject()->className()) + QLatin1Char('-') + purpose + QLatin1Char('-') + QTime::currentTime().toString();
}

QString TimeStampUpdateTest::tempFilePath(const QString& purpose) const
{
    return QDir::tempPath() + QLatin1Char('/') + tempFileName(purpose);
}

/*
 * Create a nd simulate a new startup of Digikam.
 * A new temporary database is created.
 * A collection is added, and scanned.
 */
void TimeStampUpdateTest::initTestCase()
{
    // Setup the collection folder

    QDir collectionDir = QDir(originalImageFolder);
    QVERIFY(collectionDir.exists());

    // Create new temporary database

    dbFile = tempFilePath(QLatin1String("database"));
    DbEngineParameters params(QLatin1String("QSQLITE"), dbFile, QLatin1String("QSQLITE"), dbFile);
    CoreDbAccess::setParameters(params, CoreDbAccess::MainApplication);
    QVERIFY(CoreDbAccess::checkReadyForUse(nullptr));
    QVERIFY(QFile(dbFile).exists());

    // Add collection and scan

    CollectionManager::instance()->addLocation(QUrl::fromLocalFile(collectionDir.path()));
    CollectionScanner().completeScan();

    // Verify that the scanned collection is correct

    QList<AlbumShortInfo> albums = CoreDbAccess().db()->getAlbumShortInfos();
    QVERIFY(albums.size() == 1);
    QStringList readOnlyImages;

    Q_FOREACH (const AlbumShortInfo& album, albums)
    {
        readOnlyImages << CoreDbAccess().db()->getItemURLsInAlbum(album.id);
    }

    Q_FOREACH (const QString& file, readOnlyImages)
    {
        ids << ItemInfo::fromLocalFile(file).id();
    }

    QVERIFY(!ids.contains(-1));
    QVERIFY(ids.size() == 1);
}

/*
 * Remove the database file
 */
void TimeStampUpdateTest::cleanupTestCase()
{
    QFile(dbFile).remove();
}

/*
 * Re-set the database and image file to it's original metadata state
 */
void TimeStampUpdateTest::cleanup()
{
    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setMetadataWritingMode(MetaEngine::WRITE_TO_FILE_ONLY);
    meta->setUpdateFileTimeStamp(true);
    meta->load(originalImageFile);
    meta->removeExifTag("Exif.Image.Model");
    QVERIFY2(meta->applyChanges(), "Exif.Image.Model is removed");
    QVERIFY(meta->getExifTagString("Exif.Image.Model").isEmpty());

    CollectionScanner().scanFile(originalImageFile, CollectionScanner::Rescan);

    // Check that Exif.Image.Model in database is empty

    QVariantList dbModel = CoreDbAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY2(dbModel.at(0).toString().isEmpty(), "Exif.Image.Model should be empty");
}

/*
 * This test manipulates the Exif.Image.Model without updating
 * the database.
 * A CollectionScanner().completeScan() is then launched, simulating a
 * startup of Digikam.
 * The test verifies that the change in the file is detected and
 * that new value of Exif.Image.Model is read into the database.
 */
void TimeStampUpdateTest::testRescanImageIfModifiedSet2True()
{
    // Setup metadata settings

    MetaEngineSettingsContainer set;
    set.updateFileTimeStamp   = true; // Default value
    set.rescanImageIfModified = true;
    MetaEngineSettings::instance()->setSettings(set);

    // Load the test image and verify that it's there

    QFileInfo originalFileInfo(originalImageFile);
    QVERIFY(originalFileInfo.isReadable());

    // Check that Exif.Image.Model in database is empty

    QVariantList dbModel = CoreDbAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY2(dbModel.at(0).toString().isEmpty(), "Exif.Image.Model should be empty");

    // Verify that Exif.Image.Model in image file is empty

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setMetadataWritingMode(MetaEngine::WRITE_TO_FILE_ONLY);
    meta->setUpdateFileTimeStamp(true);
    meta->load(originalImageFile);
    QString model = meta->getExifTagString("Exif.Image.Model");
    QVERIFY(model.isEmpty());

    // Change the metadata in image file

    meta->setExifTagString("Exif.Image.Model", QLatin1String("TimeStampUpdateTestCamera"));
    QVERIFY2(meta->applyChanges(), "Exif.Image.Model is added");
    QVERIFY(meta->getExifTagString("Exif.Image.Model") == QLatin1String("TimeStampUpdateTestCamera"));

    // Simulate restart of Digikam
    // The scan should detect that image file has changed

    CollectionScanner().completeScan();

    // Verify that the change is detected, and no exists in the database

    dbModel = CoreDbAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY(dbModel.at(0).toString() == QLatin1String("TimeStampUpdateTestCamera"));
}

/*
 * This test manipulates the Exif.Image.Model without updating
 * the database.
 * A CollectionScanner().completeScan() is then launched, simulating a
 * startup of Digikam.
 * The test verifies that the change in the file is disregarded and
 * that the value of Exif.Image.Model is unchanged the database.
 */
void TimeStampUpdateTest::testRescanImageIfModifiedSet2False()
{
    // Setup metadata settings

    MetaEngineSettingsContainer set;
    set.updateFileTimeStamp   = true;  // Default value
    set.rescanImageIfModified = false; // Default value
    MetaEngineSettings::instance()->setSettings(set);

    // Load the test image and verify that it's there

    QFileInfo originalFileInfo(originalImageFile);
    QVERIFY(originalFileInfo.isReadable());

    // Check that Exif.Image.Model in database is empty

    QVariantList dbModel = CoreDbAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY2(dbModel.at(0).toString().isEmpty(), "Exif.Image.Model should be empty");

    // Verify that Exif.Image.Model in image file is empty

    QScopedPointer<DMetadata> meta(new DMetadata);
    meta->setMetadataWritingMode(MetaEngine::WRITE_TO_FILE_ONLY);
    meta->setUpdateFileTimeStamp(true);
    meta->load(originalImageFile);
    QString model = meta->getExifTagString("Exif.Image.Model");
    QVERIFY(model.isEmpty());

    // Change the metadata in image file

    meta->setExifTagString("Exif.Image.Model", QLatin1String("TimeStampUpdateTestCamera"));
    QVERIFY2(meta->applyChanges(), "Exif.Image.Model is added");
    QVERIFY(meta->getExifTagString("Exif.Image.Model") == QLatin1String("TimeStampUpdateTestCamera"));

    // Simulate restart of Digikam
    // The scan should detect that image file has changed

    CollectionScanner().completeScan();

    // Verify that the changed image did not change the database

    dbModel = CoreDbAccess().db()->getImageMetadata(ids[0], DatabaseFields::Model);
    QVERIFY(dbModel.at(0).toString().isEmpty());
}
