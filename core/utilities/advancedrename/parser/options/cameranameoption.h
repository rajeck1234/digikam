/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide camera information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAMERA_NAME_OPTION_H
#define DIGIKAM_CAMERA_NAME_OPTION_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "option.h"

namespace Digikam
{

class CameraNameOption : public Option
{
    Q_OBJECT

public:

    explicit CameraNameOption();
    ~CameraNameOption()                         override = default;

protected:

    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private:

    // Disable
    CameraNameOption(QObject*)                           = delete;
    CameraNameOption(const CameraNameOption&)            = delete;
    CameraNameOption& operator=(const CameraNameOption&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_CAMERA_NAME_OPTION_H
