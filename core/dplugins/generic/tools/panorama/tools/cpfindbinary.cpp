/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpfind binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "cpfindbinary.h"

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericPanoramaPlugin
{

bool CPFindBinary::parseHeader(const QString& output)
{
    QStringList lines    = output.split(QLatin1Char('\n'));
    m_developmentVersion = false;

    Q_FOREACH (const QString& line, lines)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << path() << " help header line: \n" << line;
        QRegularExpressionMatch regMatch = headerRegExp.match(line);

        if (regMatch.hasMatch())
        {
            m_version = regMatch.captured(2);

            if (!regMatch.captured(1).isEmpty())
            {
                m_developmentVersion = true;
            }

            return true;
        }

        m_developmentVersion = true;
    }

    return false;
}

} // namespace DigikamGenericPanoramaPlugin

