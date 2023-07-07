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

#include "detectaesthetic_utest.h"

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
#include "digikam_debug.h"

QTEST_MAIN(ImgQSortTestDetectQesthetic)

ImgQSortTestDetectQesthetic::ImgQSortTestDetectQesthetic(QObject* const parent)
    : ImgQSortTest(parent)
{
    m_dataTestCases = dataTestCases;
}

void ImgQSortTestDetectQesthetic::testParseTestImagesForAestheticDetection()
{
   QHash<QString, bool> results = testParseTestImages(QLatin1String("aestheticDetection"),
                                                      ImgQSortTest_ParseTestImagesDefautDetection, DETECTAESTHETIC);

    for (const auto& test_case : results.keys())
    {
        QVERIFY(results.value(test_case));
    }
}
