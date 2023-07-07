/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : Test for SimpleTreeModel.
 *
 * SPDX-FileCopyrightText: 2010      by Michael G. Hansen <mike at mghansen dot de>
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SIMPLETREEMODEL_UTEST_H
#define DIGIKAM_SIMPLETREEMODEL_UTEST_H

// Qt includes

#include <QTest>

class TestSimpleTreeModel : public QObject
{
    Q_OBJECT

public:

    explicit TestSimpleTreeModel(QObject* const parent = nullptr)
        : QObject(parent)
    {
    }

private Q_SLOTS:

    void testNoOp();
    void testModel1();
};

#endif // DIGIKAM_SIMPLETREEMODEL_UTEST_H

