/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide date information to the parser
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DATE_OPTION_H
#define DIGIKAM_DATE_OPTION_H

// Qt includes

#include <QDialog>
#include <QObject>
#include <QString>
#include <QVariant>

// Local includes

#include "option.h"
#include "ruledialog.h"

namespace Ui
{
class DateOptionDialogWidget;
}

namespace Digikam
{

class DateFormat
{
public:

    enum Type
    {
        Standard = 0,
        ISO,
        FullText,
        UnixTimeStamp,
        Custom
    };

public:

    typedef QPair<QString, QVariant>    DateFormatDescriptor;
    typedef QList<DateFormatDescriptor> DateFormatMap;

public:

    explicit DateFormat();
    ~DateFormat() {};

    Type     type(const QString& identifier);

    QString  identifier(Type type);

    QVariant format(Type type);
    QVariant format(const QString& identifier);

    DateFormatMap& map()
    {
        return m_map;
    };

private:

    DateFormat(const DateFormat&)            = delete;
    DateFormat& operator=(const DateFormat&) = delete;

private:

    DateFormatMap m_map;
};

// --------------------------------------------------------

class DateOptionDialog : public RuleDialog
{
    Q_OBJECT

public:

    enum DateSource
    {
        FromImage = 0,
        CurrentDateTime,
        FixedDateTime
    };

public:

    explicit DateOptionDialog(Rule* parent);
    ~DateOptionDialog() override;

    Ui::DateOptionDialogWidget* const ui;

    DateSource dateSource();

private Q_SLOTS:

    void slotDateSourceChanged(int);
    void slotDateFormatChanged(int);
    void slotCustomFormatChanged(const QString&);

private:

    // Disable
    explicit DateOptionDialog(QWidget*) = delete;
    DateOptionDialog(const DateOptionDialog&);
    DateOptionDialog& operator=(const DateOptionDialog&);

    QString formattedDateTime(const QDateTime& date);
    void    updateExampleLabel();
};

// --------------------------------------------------------

class DateOption : public Option
{
    Q_OBJECT

public:

    explicit DateOption();
    ~DateOption()                                                override = default;

protected:

    QString parseOperation(ParseSettings& settings,
                           const QRegularExpressionMatch& match) override;

private Q_SLOTS:

    void slotTokenTriggered(const QString& token)                override;

private:

    // Disable
    explicit DateOption(QObject*)            = delete;
    DateOption(const DateOption&)            = delete;
    DateOption& operator=(const DateOption&) = delete;
};

} // namespace Digikam

#endif // DIGIKAM_DATE_OPTION_H
