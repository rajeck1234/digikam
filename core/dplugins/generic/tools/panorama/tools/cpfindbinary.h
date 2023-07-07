/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-05-23
 * Description : Autodetects cpfind binary program and version
 *
 * SPDX-FileCopyrightText: 2011-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CPFIND_BINARY_H
#define DIGIKAM_CPFIND_BINARY_H

// Qt includes

#include <QRegularExpression>

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class CPFindBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit CPFindBinary(QObject* const parent = nullptr)
        : DBinaryIface(QLatin1String("cpfind"),
                       QLatin1String("2010.4"),
                       QString(),
                       0,
                       QLatin1String("Hugin"),
                       QLatin1String("http://hugin.sourceforge.net/download/"),     // krazy:exclude=insecurenet
                       QLatin1String("Panorama"),
                       QStringList(QLatin1String("--version"))
                      ),
          headerRegExp(QLatin1String("^Hugin'?s cpfind( Pre-Release)? (\\d+\\.\\d+(\\.\\d+)?)(\\D.*)?$"))
        {
            Q_UNUSED(parent);
            setup();
        }

    ~CPFindBinary() override
    {
    }

protected:

    bool parseHeader(const QString& output) override;

private:

    QRegularExpression headerRegExp;
};

} // namespace DigikamGenericPanoramaPlugin

#endif // DIGIKAM_CPFIND_BINARY_H
