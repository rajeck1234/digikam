/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-02-04
 * Description : a tool to create panorama by fusion of several images.
 *               This parser is based on pto file format described here:
 *               hugin.sourceforge.net/docs/nona/nona.txt, and
 *               on pto files produced by Hugin's tools.
 *
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_PTO_FILE_H
#define DIGIKAM_PTO_FILE_H

// Qt includes

#include <QString>

// Local includes

#include "ptotype.h"

namespace Digikam
{

class PTOFile
{
public:

    explicit PTOFile(const QString& huginVersion);
    ~PTOFile();

    bool     openFile(const QString& path);
    PTOType* getPTO();

private:

    // Disable
    PTOFile(const PTOFile&)            = delete;
    PTOFile& operator=(const PTOFile&) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_PTO_FILE_H
