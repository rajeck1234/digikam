/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a class to build the tooltip for a renameparser and its options
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TOOL_TIPCREATOR_H
#define DIGIKAM_TOOL_TIPCREATOR_H

// Qt includes

#include <QString>

// Local includes

#include "rule.h"

namespace Digikam
{

class Parser;

class TooltipCreator
{
public:

    ~TooltipCreator()
    {
    };

    static TooltipCreator& getInstance();

    QString tooltip(Parser* parser);

private:

    QIcon getInfoIcon()         const;
    QString getInfoIconBase64() const;

    /// common methods

    QString markOption(const QString& str);
    QString tableStart();
    QString tableStart(int widthPercentage);
    QString tableEnd();

    QString additionalInformation();

    /// parse object related methods

    QString createEntries(const RulesList& data);
    QString createSection(const QString& sectionName, const RulesList& data, bool lastSection = false);
    QString createHeader(const QString& str);

private:

    // Disable
    explicit TooltipCreator()                        = default;
    TooltipCreator(const TooltipCreator&)            = delete;
    TooltipCreator& operator=(const TooltipCreator&) = delete;

};

} // namespace Digikam

#endif // DIGIKAM_TOOL_TIPCREATOR_H
