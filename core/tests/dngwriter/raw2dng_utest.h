/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-02-17
 * Description : an unit-test to convert RAW file to DNG
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RAW2DNG_UTEST_H
#define DIGIKAM_RAW2DNG_UTEST_H

// Qt includes

#include <QString>
#include <QObject>

class Raw2DngTest : public QObject
{
    Q_OBJECT

public:

    explicit Raw2DngTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testRaw2Dng();
};

#endif // DIGIKAM_RAW2DNG_UTEST_H
