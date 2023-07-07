/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-05-01
 * Description : a dialog that can be used to display a configuration
 *               dialog for a rule
 *
 * SPDX-FileCopyrightText: 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RULE_DIALOG_H
#define DIGIKAM_RULE_DIALOG_H

// Qt includes

#include <QDialog>

namespace Digikam
{

class Rule;

class RuleDialog : public QDialog
{
    Q_OBJECT

public:

    explicit RuleDialog(Rule* const parent);
    ~RuleDialog() override;

    void setSettingsWidget(QWidget* const settingsWidget);

private:

    void setDialogTitle(const QString& title);
    void setDialogDescription(const QString& description);
    void setDialogIcon(const QPixmap& pixmap);

private:

    // Disable
    explicit RuleDialog(QWidget*)            = delete;
    RuleDialog(const RuleDialog&)            = delete;
    RuleDialog& operator=(const RuleDialog&) = delete;

private Q_SLOTS:

    void slotHelp();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_RULE_DIALOG_H
