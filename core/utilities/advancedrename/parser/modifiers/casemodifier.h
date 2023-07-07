/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : modifier to change the case of a renaming option
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CASE_MODIFIER_H
#define DIGIKAM_CASE_MODIFIER_H

// Local includes

#include "modifier.h"

namespace Digikam
{

class CaseModifier : public Modifier
{
    Q_OBJECT

public:

    explicit CaseModifier();
    QString parseOperation(ParseSettings& settings, const QRegularExpressionMatch& match) override;

private:

    // Disable
    explicit CaseModifier(QObject*)              = delete;
    CaseModifier(const CaseModifier&)            = delete;
    CaseModifier& operator=(const CaseModifier&) = delete;

private:

    QString firstupper(const QString& str2Modify);
};

} // namespace Digikam

#endif // DIGIKAM_CASE_MODIFIER_H
