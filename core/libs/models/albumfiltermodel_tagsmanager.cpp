/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-03-24
 * Description : Qt Model for Albums - filter model
 *
 * SPDX-FileCopyrightText: 2008-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2009      by Johannes Wienke <languitar at semipol dot de>
 * SPDX-FileCopyrightText: 2014-2015 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "albumfiltermodel_p.h"

namespace Digikam
{

TagsManagerFilterModel::TagsManagerFilterModel(QObject* const parent)
    : TagPropertiesFilterModel(parent)
{
}

void TagsManagerFilterModel::setQuickListTags(const QList<int>& tags)
{
    m_keywords.clear();

    Q_FOREACH (int tag, tags)
    {
        m_keywords << tag;
    }

    invalidateFilter();
    Q_EMIT signalFilterChanged();
}

bool TagsManagerFilterModel::matches(Album* album) const
{
    if (!TagPropertiesFilterModel::matches(album))
    {
        return false;
    }

    if (m_keywords.isEmpty())
    {
        return true;
    }

    bool dirty = false;

    for (QSet<int>::const_iterator it = m_keywords.begin() ;
         it != m_keywords.end() ; ++it)
    {
        TAlbum* const talbum = AlbumManager::instance()->findTAlbum(*it);

        if (!talbum)
        {
            continue;
        }

        if (talbum->title().compare(album->title()) == 0)
        {
            dirty = true;
        }
    }

    return dirty;
}

} // namespace Digikam
