/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2014-09-30
 * Description : a tool to export items to Piwigo web service
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006      by Colin Guthrie <kde at colin dot guthr dot ie>
 * SPDX-FileCopyrightText: 2006-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008      by Andrea Diamantini <adjam7 at gmail dot com>
 * SPDX-FileCopyrightText: 2010-2014 by Frederic Coiffier <frederic dot coiffier at free dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_PIWIGO_ITEM_H
#define DIGIKAM_PIWIGO_ITEM_H

// Qt includes

#include <QString>

namespace DigikamGenericPiwigoPlugin
{

class PiwigoAlbum
{
public:

    explicit PiwigoAlbum()
      : m_refNum      (-1),
        m_parentRefNum(-1)
    {
    }

    bool operator<(const PiwigoAlbum& rhs) const
    {
        if (m_parentRefNum == rhs.m_parentRefNum)
        {
            return (m_refNum < rhs.m_refNum);
        }

        return (m_parentRefNum < rhs.m_parentRefNum);
    }

public:

    int     m_refNum;         ///< album reference number
    int     m_parentRefNum;   ///< parent's album reference number

    QString m_name;           ///< Album name
};

} // namespace DigikamGenericPiwigoPlugin

#endif // DIGIKAM_PIWIGO_ITEM_H
