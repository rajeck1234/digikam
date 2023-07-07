/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : An unit test to load metadata from byte array
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_LOAD_FROM_BA_UTEST_H
#define DIGIKAM_LOAD_FROM_BA_UTEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

using namespace Digikam;

class LoadFromBATest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit LoadFromBATest(QObject* const parent = nullptr);

private:

    void loadFromByteArray(const QString& file);

private Q_SLOTS:

    void testLoadFromByteArray();
};

#endif // DIGIKAM_LOAD_FROM_BA_UTEST_H
