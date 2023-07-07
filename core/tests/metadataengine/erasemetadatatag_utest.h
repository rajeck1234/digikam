/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-06-11
 * Description : an unit-test tool to erase metadata tag from photo with DMetadata
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ERASE_METADATA_TAG_UTEST_H
#define DIGIKAM_ERASE_METADATA_TAG_UTEST_H

// Local includes

#include "abstractunittest.h"
#include "metaenginesettingscontainer.h"
#include "dmetadatasettingscontainer.h"

using namespace Digikam;

class EraseMetadataTagTest : public AbstractUnitTest
{
    Q_OBJECT

public:

    explicit EraseMetadataTagTest(QObject* const parent = nullptr);

private:

    void eraseMetadataTag(const QString& file,
                          const QString& metadataTag,
                          const MetaEngineSettingsContainer& settings);

private Q_SLOTS:

    void testEraseMetadataTag();
};

#endif // DIGIKAM_ERASE_METADATA_TAG_UTEST_H
