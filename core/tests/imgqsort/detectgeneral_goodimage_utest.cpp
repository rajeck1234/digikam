/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : an unit-test to detect image quality level
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "detectgeneral_goodimage_utest.h"

// Qt includes

#include <QTest>
#include <QStringList>
#include <QFileInfoList>
#include <QDebug>
#include <QDir>

// Local includes

#include "digikam_globals.h"
#include "imagequalitycontainer.h"
#include "dpluginloader.h"

using namespace Digikam;

QTEST_MAIN(ImgQSortTestGeneral)

ImgQSortTestGeneral::ImgQSortTestGeneral(QObject* const)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestGeneral::testParseTestImagesForGeneralQuiteGoodImage()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("quiteGoodImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTIONGENERAL);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}

void ImgQSortTestGeneral::testParseTestImagesForGeneralGoodImage()
{
    QHash<QString, bool> results = testParseTestImages(QLatin1String("goodImage"),
                                                       ImgQSortTest_ParseTestImagesDefautDetection, DETECTIONGENERAL);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}
