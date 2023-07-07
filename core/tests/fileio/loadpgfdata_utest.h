/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-09
 * Description : an unit-test to load PGF data and convert to QImage
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LOAD_PGF_DATA_UTEST_H
#define DIGIKAM_LOAD_PGF_DATA_UTEST_H

// Qt includes

#include <QObject>

class LoadPGFDataTest : public QObject
{
    Q_OBJECT

public:

    explicit LoadPGFDataTest(QObject* const parent = nullptr);

private Q_SLOTS:

    void testLoadData();
};

#endif // DIGIKAM_LOAD_PGF_DATA_UTEST_H
