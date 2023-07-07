/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-11-02
 * Description : Implementation of grouping specific functions for views
 *
 * SPDX-FileCopyrightText: 2017 by Simon Frei <freisim93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_GROUPING_VIEW_IMPLEMENTATION_H
#define DIGIKAM_GROUPING_VIEW_IMPLEMENTATION_H

// Local includes

#include "applicationsettings.h"
#include "digikam_export.h"

namespace Digikam
{

class ItemInfo;
class ItemInfoList;

class DIGIKAM_GUI_EXPORT GroupingViewImplementation
{

public:

    GroupingViewImplementation()                                                = default;
    virtual ~GroupingViewImplementation()                                       = default;

    /// must be implemented by parent view

    virtual bool  hasHiddenGroupedImages(const ItemInfo&)                       const
    {
        return false;
    }

    bool          needGroupResolving(ApplicationSettings::OperationType type,
                                     const ItemInfoList& infos)                 const;

    ItemInfoList resolveGrouping(const ItemInfoList& infos)                     const;
    ItemInfoList getHiddenGroupedInfos(const ItemInfoList& infos)               const;

private:

    Q_DISABLE_COPY(GroupingViewImplementation)
};

} // namespace Digikam

#endif // DIGIKAM_GROUPING_VIEW_IMPLEMENTATION_H
