/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-11-24
 * Description : Batch Tool utils.
 *
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "batchtoolutils.h"

// Qt includes

#include <QFileInfo>

// Local includes

#include "batchtool.h"
#include "batchtoolsfactory.h"

namespace Digikam
{

BatchToolSet::BatchToolSet()
    : index  (-1),
      version(0),
      group  (BatchTool::BaseTool)
{
}

bool BatchToolSet::operator==(const BatchToolSet& set) const
{
    return (
             (index   == set.index)   &&
             (version == set.version) &&
             (name    == set.name )   &&
             (group   == set.group)
           );
}

QDebug operator<<(QDebug dbg, const BatchToolSet& s)
{
    dbg.nospace() << "BatchToolSet::";
    dbg.nospace() << "index: "    << s.index   << ", ";
    dbg.nospace() << "version: "  << s.version << ", ";
    dbg.nospace() << "name: "     << s.name    << ", ";
    dbg.nospace() << "group: "    << s.group   << ", ";
    dbg.nospace() << "settings: " << s.settings;

    return dbg.space();
}

// ---------------------------------------------------------------------------------------------

QString AssignedBatchTools::targetSuffix(bool* const extSet) const
{
    QString suffix;

    Q_FOREACH (const BatchToolSet& set, m_toolsList)
    {
        BatchTool* const tool = BatchToolsFactory::instance()->findTool(set.name, set.group);

        if (tool)
        {
            QString s = tool->outputSuffix();

            if (!s.isEmpty())
            {
                suffix = s;

                if (extSet != nullptr)
                {
                    *extSet = true;
                }
            }
        }
    }

    if (suffix.isEmpty())
    {
        if (extSet != nullptr)
        {
            *extSet = false;
        }

        return (QFileInfo(m_itemUrl.fileName()).suffix());
    }

    return suffix;
}

} // namespace Digikam
