/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : Autodetect align_image_stack binary program and version
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2012-2015 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ALIGN_BINARY_H
#define DIGIKAM_ALIGN_BINARY_H

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericExpoBlendingPlugin
{

class AlignBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit AlignBinary();
    ~AlignBinary() override;

private:

    // Disable
    explicit AlignBinary(QObject*) = delete;
};

} // namespace DigikamGenericExpoBlendingPlugin

#endif // DIGIKAM_ALIGN_BINARY_H
