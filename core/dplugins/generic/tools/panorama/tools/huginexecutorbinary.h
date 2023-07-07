/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-11-02
 * Description : Autodetect hugin_executor binary program and version
 *
 * SPDX-FileCopyrightText: 2015-2016 by Benjamin Girault <benjamin dot girault at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_HUGIN_EXECUTOR_BINARY_H
#define DIGIKAM_HUGIN_EXECUTOR_BINARY_H

// Local includes

#include "dbinaryiface.h"

using namespace Digikam;

namespace DigikamGenericPanoramaPlugin
{

class HuginExecutorBinary : public DBinaryIface
{
    Q_OBJECT

public:

    explicit HuginExecutorBinary(QObject* const parent = nullptr);
    ~HuginExecutorBinary() override;
};

} // namespace DigikamGenericPanoramaPlugin

#endif // DIGIKAM_HUGIN_EXECUTOR_BINARY_H
