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

#ifndef DIGIKAM_IMGQSORT_TEST_H
#define DIGIKAM_IMGQSORT_TEST_H

// Qt includes

#include <QObject>
#include <QDir>
#include <QMultiMap>
#include <QString>

// Local includes

#include "imgqsorttest_shared.h"
#include "digikam_globals.h"
#include "digikam_debug.h"
#include "imagequalitycontainer.h"
#include "exiftoolparser.h"
#include "dpluginloader.h"
#include "dtestdatadir.h"

// Shared class for utest

using namespace Digikam;

using PairImageQuality = QPair<QString, int>;
using DataTestCases    = QMultiMap<QString, PairImageQuality>;

class ImgQSortTest : public QObject
{
    Q_OBJECT

public:

    explicit ImgQSortTest(QObject* const parent = nullptr)
        : QObject      (parent),
          m_hasExifTool(false)
    {
    }

protected:

    QDir imageDir() const
    {
        QDir dir = DTestDataDir::TestData(QString::fromUtf8("core/tests/imgqsort")).root();
        qCDebug(DIGIKAM_TESTS_LOG) << "Test Data Dir:" << dir;

        return dir;
    }

    template <typename Function, typename Parameter>
    QHash<QString, bool> testParseTestImages(const QString& testcase_name, Function ParseTestFunc, Parameter parameter)
    {
        QStringList imageNames;

        QList<PairImageQuality> dataTest = getDataTestCases().values(testcase_name);

        for (const auto& image_refQuality : dataTest)
        {
            imageNames << image_refQuality.first;
        }

        QFileInfoList list                    = imageDir().entryInfoList(imageNames, QDir::Files, QDir::Name);
        QHash<QString, int> results_detection = ParseTestFunc(parameter, list);
        QHash<QString, bool> results_test;

        for (const auto& image_refQuality : dataTest)
        {
            results_test.insert(image_refQuality.first, results_detection.value(image_refQuality.first) == image_refQuality.second);
        }

        return results_test;
    }

    DataTestCases getDataTestCases() const
    {
        return m_dataTestCases;
    }

protected Q_SLOTS:

    void initTestCase()
    {
        QDir dir(qApp->applicationDirPath());
        qputenv("DK_PLUGIN_PATH", dir.canonicalPath().toUtf8());
        DPluginLoader::instance()->init();

        QScopedPointer<ExifToolParser> const parser(new ExifToolParser(nullptr));
        m_hasExifTool = parser->exifToolAvailable();
    }

    void cleanupTestCase()
    {
        DPluginLoader::instance()->cleanUp();
    }

protected:

    DataTestCases m_dataTestCases;
    bool          m_hasExifTool;
};

#endif // DIGIKAM_IMGQSORT_TEST_H
