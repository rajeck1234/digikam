/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-11-19
 * Description : syntax highlighter for AdvancedRename utility
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "highlighter.h"

// Qt includes

#include <QTextDocument>

// Local includes

#include "parser.h"

namespace Digikam
{

Highlighter::Highlighter(QTextDocument* const document, Parser* const _parser)
    : QSyntaxHighlighter(document),
      parser            (_parser)

{
    setupHighlightingGrammar();
}

Highlighter::~Highlighter()
{
}

void Highlighter::highlightBlock(const QString& text)
{
    Q_FOREACH (const HighlightingRule& rule, highlightingRules)
    {
        QRegularExpression      expression(rule.pattern);
        QRegularExpressionMatch match;
        int index = text.indexOf(expression, 0, &match);

        while (index >= 0)
        {
            int length = match.capturedLength();
            setFormat(index, length, rule.format);

            switch (rule.type)
            {
                case OptionPattern:
                case ModifierPattern:
                {
                    // highlight parameters in options and modifiers

                    if ((expression.captureCount() > 0) && !match.captured(1).isEmpty())
                    {
                        QString fullmatched  = match.captured(0);
                        QString parameters   = match.captured(1);

                        if (parameters.startsWith(QLatin1Char(':')))
                        {
                            parameters.remove(0, 1);

                            if (!parameters.isEmpty())
                            {
                                int pindex = fullmatched.indexOf(parameters);

                                while (pindex >= 0)
                                {
                                    int plength = parameters.length();
                                    setFormat(index + pindex, plength, parameterFormat);
                                    pindex      = fullmatched.indexOf(parameters, pindex + plength);
                                }
                            }
                        }
                    }

                    break;
                }

                default:
                {
                    break;
                }
            }

            index = text.indexOf(expression, index+length, &match);
        }
    }

    // mark invalid modifiers in the parse string

    ParseSettings settings;
    settings.parseString = text;
    ParseResults invalid = parser->invalidModifiers(settings);

    Q_FOREACH (const ParseResults::ResultsKey& key, invalid.keys())
    {
        setFormat(key.first, key.second, errorFormat);
    }

    // highlight quoted text in options and modifiers

    {
        QRegularExpression      expression(quotationRule.pattern);
        QRegularExpressionMatch match;
        int index = text.indexOf(expression, 0, &match);

        while (index >= 0)
        {
            QString fullmatched = match.captured(0);
            int qlength         = match.capturedLength();
            setFormat(index, qlength, quotationFormat);
            index = text.indexOf(expression, index + qlength, &match);
        }
    }
}

void Highlighter::setupHighlightingGrammar()
{
    if (!parser)
    {
        return;
    }

    HighlightingRule rule;

    // --------------------------------------------------------

    optionFormat.setForeground(Qt::red);

    Q_FOREACH (Rule* const option, parser->options())
    {
        QRegularExpression r    = option->regExp();
        rule.type               = OptionPattern;
        rule.pattern            = r;
        rule.format             = optionFormat;
        highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    modifierFormat.setForeground(Qt::darkGreen);

    Q_FOREACH (Rule* const modifier, parser->modifiers())
    {
        QRegularExpression r    = modifier->regExp();
        rule.type               = ModifierPattern;
        rule.pattern            = r;
        rule.format             = modifierFormat;
        highlightingRules.append(rule);
    }

    // --------------------------------------------------------

    quotationFormat.setForeground(QColor(0x50, 0x50, 0xff)); // light blue
    quotationFormat.setFontItalic(true);
    quotationRule.pattern = QRegularExpression(QLatin1String("\".*\""));
    quotationRule.pattern.setPatternOptions(QRegularExpression::InvertedGreedinessOption);
    quotationRule.format  = quotationFormat;
    quotationRule.type    = QuotedTextPattern;

    // --------------------------------------------------------

    parameterFormat.setForeground(Qt::darkYellow);
    parameterFormat.setFontItalic(true);

    // --------------------------------------------------------

    errorFormat.setForeground(Qt::white);
    errorFormat.setBackground(Qt::red);
}

} // namespace Digikam
