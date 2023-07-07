/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-05-11
 * Description : setup Light Table tab.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SETUP_LIGHT_TABLE_H
#define DIGIKAM_SETUP_LIGHT_TABLE_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupLightTable : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupLightTable(QWidget* const parent = nullptr);
    ~SetupLightTable() override;

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SETUP_LIGHT_TABLE_H
