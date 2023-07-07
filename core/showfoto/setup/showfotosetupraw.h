/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup RAW decoding settings.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOW_FOTO_SETUP_RAW_H
#define SHOW_FOTO_SETUP_RAW_H

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupRaw : public QScrollArea
{
    Q_OBJECT

public:

    enum RAWTab
    {
        RAWBehavior = 0,
        RAWDefaultSettings
    };

public:

    explicit ShowfotoSetupRaw(QWidget* const parent = nullptr);
    ~ShowfotoSetupRaw() override;

    void setActiveTab(RAWTab tab);
    RAWTab activeTab() const;

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOW_FOTO_SETUP_RAW_H
