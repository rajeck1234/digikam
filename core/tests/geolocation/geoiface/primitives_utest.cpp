/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-02-07
 * Description : test for the simple datatypes and helper functions
 *
 * SPDX-FileCopyrightText: 2010-2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "primitives_utest.h"

// local includes

#include "geoifacetypes.h"
#include "geoifacecommon.h"

using namespace Digikam;

void TestPrimitives::testNoOp()
{
}

void TestPrimitives::testParseLatLonString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String("52,6"), nullptr));

    GeoCoordinates coordinate;

    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String("52,6"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String("52.5,6.5"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String(" 52.5, 6.5 "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String("-52.5, 6.5 "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:-52.5,6.5"));

    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String("    -52.5,  6.5   "), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:-52.5,6.5"));

    QVERIFY(GeoIfaceHelperParseLatLonString(QLatin1String("52.5,-6.5"), &coordinate));
    QCOMPARE(coordinate.geoUrl(), QLatin1String("geo:52.5,-6.5"));

    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String(""), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String("52.6"), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String("52.6,"), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String(",6"), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String("a52,6"), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String("52,a"), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String("52,6a"), nullptr));
    QVERIFY(!GeoIfaceHelperParseLatLonString(QLatin1String("(52,6)"), nullptr));
}

void TestPrimitives::testParseXYStringToPoint()
{
    // make sure there is no crash on null-pointer
    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String("(52,6)"), nullptr));

    QPoint point;

    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String("(52,6)"), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String("(10,20)"), &point));
    QCOMPARE(point, QPoint(10,20));

    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String(" ( 52, 6 ) "), &point));
    QCOMPARE(point, QPoint(52,6));

    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String("  ( 52, 6 )  "), &point));
    QCOMPARE(point, QPoint(52,6));

    // We used to expect integer string results, but floats are also possible.
    // BKO 270624
    // GeoIfaceHelperParseXYStringToPoint always rounds them to 0.
    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String("  ( 52.5, 6.5 )  "), &point));
    QCOMPARE(point, QPoint(52,6));
    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QLatin1String("  ( -52.5, 6.5 )  "), &point));
    QCOMPARE(point, QPoint(-52,6));

    QVERIFY(GeoIfaceHelperParseXYStringToPoint(QString::fromLatin1("(204.94641003022224, 68.00444002512285)"), &point));

    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String(""), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("()"), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("(52)"), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("(52,6a)"), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("(a52,6)"), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("52,6"), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("(,6)"), nullptr));
    QVERIFY(!GeoIfaceHelperParseXYStringToPoint(QLatin1String("(6,)"), nullptr));
}

void TestPrimitives::testParseBoundsString()
{
    // make sure there is no crash on null-pointer
    QVERIFY(GeoIfaceHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), nullptr));

    GeoCoordinates::Pair bounds;

    QVERIFY(GeoIfaceHelperParseBoundsString(QLatin1String("((-52,-6),(52,6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(GeoIfaceHelperParseBoundsString(QLatin1String("((-52,-6), (52,6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(GeoIfaceHelperParseBoundsString(QLatin1String("((-52, -6), (52, 6))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52,-6"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52,6"));

    QVERIFY(GeoIfaceHelperParseBoundsString(QLatin1String("((10,20),(30,40))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:10,20"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:30,40"));

    QVERIFY(GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5))"), &bounds));
    QCOMPARE(bounds.first.geoUrl(), QLatin1String("geo:-52.5,-6.5"));
    QCOMPARE(bounds.second.geoUrl(), QLatin1String("geo:52.5,6.5"));

    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String(" (-52.5,-6.5),(52.5,6.5))"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5) "), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,-6.5), 52.5,6.5))"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,-6.5  (52.5,6.5))"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5 -6.5),(52.5,6.5))"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5 6.5))"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("( -52.5,-6.5),(52.5,6.5))"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,-6.5),(52.5,6.5)a"), nullptr));
    QVERIFY(!GeoIfaceHelperParseBoundsString(QLatin1String("((-52.5,),(52.5,6.5))"),     nullptr));
}

void TestPrimitives::testNormalizeBounds_data()
{
    QTest::addColumn<GeoCoordinates::Pair>("bounds");
    QTest::addColumn<QList<GeoCoordinates::Pair> >("nbounds");

    // these ones should not be split:
    QTest::newRow("top-left")
        << GeoCoordinates::makePair(10, 20, 12, 22)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(10, 20, 12, 22) );

    QTest::newRow("bottom-left")
        << GeoCoordinates::makePair(-12, 20, -10, 22)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(-12, 20, -10, 22) );

    QTest::newRow("top-right")
        << GeoCoordinates::makePair(10, -22, 12, -20)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(10, -22, 12, -20) );

    QTest::newRow("bottom-right")
        << GeoCoordinates::makePair(-12, -22, -10, -20)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(-12, -22, -10, -20) );

    QTest::newRow("cross_origin")
        << GeoCoordinates::makePair(-12, -22, 10, 20)
        << ( GeoCoordinates::PairList() << GeoCoordinates::makePair(-12, -22, 10, 20) );

    // these ones should be split:
    QTest::newRow("cross_date_1")
        << GeoCoordinates::makePair(10, 20, 15, -170)
        << ( GeoCoordinates::PairList()
            << GeoCoordinates::makePair(10, -180, 15, -170)
            << GeoCoordinates::makePair(10, 20, 15, 180)
        );

    QTest::newRow("cross_date_2")
        << GeoCoordinates::makePair(-10, 20, 15, -170)
        << ( GeoCoordinates::PairList()
                << GeoCoordinates::makePair(-10, -180, 15, -170)
                << GeoCoordinates::makePair(-10, 20, 15, 180)
        );
}

void TestPrimitives::testNormalizeBounds()
{
    QFETCH(GeoCoordinates::Pair, bounds);

    QTEST(GeoIfaceHelperNormalizeBounds(bounds), "nbounds");
}

void TestPrimitives::testGroupStateComputer()
{
    {
        // test selected state:
        GroupStateComputer c1;
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedNone);
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedSome);
        QCOMPARE(c1.getState(), SelectedSome);
        c1.addSelectedState(SelectedAll);
        QCOMPARE(c1.getState(), SelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedAll);
        QCOMPARE(c1.getState(), SelectedAll);
        c1.addSelectedState(SelectedSome);
        QCOMPARE(c1.getState(), SelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), SelectedNone);
        c1.addSelectedState(SelectedAll);
        QCOMPARE(c1.getState(), SelectedAll);
        c1.addSelectedState(SelectedNone);
        QCOMPARE(c1.getState(), SelectedSome);
    }

    {
        // test selected state:
        GroupStateComputer c1;
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveNone);
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveSome);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
        c1.addFilteredPositiveState(FilteredPositiveAll);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveAll);
        QCOMPARE(c1.getState(), FilteredPositiveAll);
        c1.addFilteredPositiveState(FilteredPositiveSome);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
        c1.clear();
        QCOMPARE(c1.getState(), FilteredPositiveNone);
        c1.addFilteredPositiveState(FilteredPositiveAll);
        QCOMPARE(c1.getState(), FilteredPositiveAll);
        c1.addFilteredPositiveState(FilteredPositiveNone);
        QCOMPARE(c1.getState(), FilteredPositiveSome);
    }

    {
        // test selected state:
        GroupStateComputer c1;
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedNone);
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedSome);
        QCOMPARE(c1.getState(), RegionSelectedSome);
        c1.addRegionSelectedState(RegionSelectedAll);
        QCOMPARE(c1.getState(), RegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedAll);
        QCOMPARE(c1.getState(), RegionSelectedAll);
        c1.addRegionSelectedState(RegionSelectedSome);
        QCOMPARE(c1.getState(), RegionSelectedSome);
        c1.clear();
        QCOMPARE(c1.getState(), RegionSelectedNone);
        c1.addRegionSelectedState(RegionSelectedAll);
        QCOMPARE(c1.getState(), RegionSelectedAll);
        c1.addRegionSelectedState(RegionSelectedNone);
        QCOMPARE(c1.getState(), RegionSelectedSome);
    }

    /// @todo Test addState
}

QTEST_GUILESS_MAIN(TestPrimitives)
