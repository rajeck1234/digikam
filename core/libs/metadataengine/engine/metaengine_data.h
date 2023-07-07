/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-09-15
 * Description : Exiv2 library interface.
 *               Shared data container.
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef META_ENGINE_DATA_H
#define META_ENGINE_DATA_H

// Qt includes

#include <QExplicitlySharedDataPointer>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT MetaEngineData
{
public:

    MetaEngineData();
    MetaEngineData(const MetaEngineData&);
    ~MetaEngineData();

    MetaEngineData& operator=(const MetaEngineData&);

public:

    // Declared as public due to use in MetaEngine::Private class
    class Private;

private:

    QExplicitlySharedDataPointer<Private> d;

    friend class MetaEngine;
};

} // namespace Digikam

#endif // META_ENGINE_DATA_H
