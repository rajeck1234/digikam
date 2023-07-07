/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-10-30
 * Description : An unit-test to read/write metadata from XMP sidecar with DMetadata.
 *
 * SPDX-FileCopyrightText: 2019-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_USE_XMP_SIDECAR_UTEST_H
#define DIGIKAM_USE_XMP_SIDECAR_UTEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

class UseXmpSidecarTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit UseXmpSidecarTest(QObject* const parent = nullptr);

private:

    void useXmpSidecar(const QString& file,
                       const MetaEngineSettingsContainer& settings);

private Q_SLOTS:

    void testUseXmpSidecar();
};

#endif // DIGIKAM_USE_XMP_SIDECAR_UTEST_H
