/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-07-18
 * Description : setup Metadata tab.
 *
 * SPDX-FileCopyrightText: 2009-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOW_FOTO_SETUP_METADATA_H
#define SHOW_FOTO_SETUP_METADATA_H

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class ShowfotoSetupMetadata : public QScrollArea
{
    Q_OBJECT

public:

    enum MetadataTab
    {
        Behavior = 0,
        ExifViewer,
        MakernotesViewer,
        IptcViewer,
        XmpViewer,
        ExifTool
    };

public:

    explicit ShowfotoSetupMetadata(QWidget* const parent = nullptr);
    ~ShowfotoSetupMetadata() override;

    void applySettings();

    void setActiveTab(MetadataTab tab);
    MetadataTab activeTab() const;

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOW_FOTO_SETUP_METADATA_H
