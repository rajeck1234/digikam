/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-09-16
 * Description : Dialog to prompt users about versioning
 *
 * SPDX-FileCopyrightText: 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_VERSIONING_PROMPT_USER_SAVE_DLG_H
#define DIGIKAM_VERSIONING_PROMPT_USER_SAVE_DLG_H

// Qt includes

#include <QDialog>

class QAbstractButton;

namespace Digikam
{

class VersioningPromptUserSaveDialog : public QDialog
{
    Q_OBJECT

public:

    explicit VersioningPromptUserSaveDialog(QWidget* const parent);
    ~VersioningPromptUserSaveDialog() override;

    bool shallSave()    const;
    bool newVersion()   const;
    bool shallDiscard() const;

private Q_SLOTS:

    void slotButtonClicked(QAbstractButton*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_VERSIONING_PROMPT_USER_SAVE_DLG_H
