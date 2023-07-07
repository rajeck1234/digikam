/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-07-09
 * Description : item tool tip configuration setup tab
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SETUP_TOOLTIP_H
#define DIGIKAM_SETUP_TOOLTIP_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupToolTip : public QScrollArea
{
    Q_OBJECT

public:

    enum ToolTipTab
    {
        IconItems = 0,
        AlbumItems,
        ImportItems
    };

public:

    explicit SetupToolTip(QWidget* const parent = nullptr);
    ~SetupToolTip() override;

    void setActiveTab(ToolTipTab tab);
    ToolTipTab activeTab() const;

    void applySettings();

public Q_SLOTS:

    void slotUseFileMetadataChanged(bool);

private Q_SLOTS:

    void slotImportToolTipsChanged();

private:

    void readSettings();
    void refreshCameraOptions();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_SETUP_TOOLTIP_H
