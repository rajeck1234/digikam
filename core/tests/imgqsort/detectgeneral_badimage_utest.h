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

#ifndef DIGIKAM_DETECT_GENERAL_BAD_IMAGE_UTEST_H
#define DIGIKAM_DETECT_GENERAL_BAD_IMAGE_UTEST_H

// Qt includes

#include <QObject>
#include <QDir>
#include <QMultiMap>
#include <QString>

// Local includes

#include "imgqsorttest.h"

class ImgQSortTestGeneral : public ImgQSortTest
{
    Q_OBJECT

public:

    explicit ImgQSortTestGeneral(QObject* const parent = nullptr);

private Q_SLOTS:

    void testParseTestImagesForGeneralBadImage();
    void testParseTestImagesForGeneralQuiteBadImage();
};

DataTestCases const dataTestCases =
{
    {QLatin1String("badImage"),         PairImageQuality(QLatin1String("general_bad_image_1.jpg"),       1)},
    {QLatin1String("badImage"),         PairImageQuality(QLatin1String("general_bad_image_3.jpg"),       1)},
    {QLatin1String("badImage"),         PairImageQuality(QLatin1String("general_bad_image_5.jpg"),       1)},
    {QLatin1String("badImage"),         PairImageQuality(QLatin1String("general_bad_image_6.jpg"),       1)},

    {QLatin1String("quiteBadImage"),    PairImageQuality(QLatin1String("general_quite_bad_image_1.jpg"), 1)},
    {QLatin1String("quiteBadImage"),    PairImageQuality(QLatin1String("general_quite_bad_image_2.jpg"), 1)},
    {QLatin1String("quiteBadImage"),    PairImageQuality(QLatin1String("general_quite_bad_image_3.jpg"), 1)},
    {QLatin1String("quiteBadImage"),    PairImageQuality(QLatin1String("general_quite_bad_image_4.jpg"), 1)},
    {QLatin1String("quiteBadImage"),    PairImageQuality(QLatin1String("general_quite_bad_image_5.jpg"), 1)},
};

#endif // DIGIKAM_DETECT_GENERAL_BAD_IMAGE_UTEST_H
