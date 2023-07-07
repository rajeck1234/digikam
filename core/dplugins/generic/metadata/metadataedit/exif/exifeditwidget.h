/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-14
 * Description : a Widget to edit EXIF metadata
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2011 by Victor Dodon <dodon dot victor at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_EXIF_EDIT_WIDGET_H
#define DIGIKAM_EXIF_EDIT_WIDGET_H

// Local includes

#include "dconfigdlgwidgets.h"

using namespace Digikam;

namespace DigikamGenericMetadataEditPlugin
{

class MetadataEditDialog;

class EXIFEditWidget : public DConfigDlgWdg
{
    Q_OBJECT

public:

    explicit EXIFEditWidget(MetadataEditDialog* const parent);
    ~EXIFEditWidget() override;

    bool isModified() const;
    void saveSettings();
    void apply();

Q_SIGNALS:

    void signalSetReadOnly(bool);
    void signalModified();

public Q_SLOTS:

    void slotModified();
    void slotItemChanged();

private:

    void readSettings();

    int  activePageIndex() const;
    void showPage(int page);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericMetadataEditPlugin

#endif // DIGIKAM_EXIF_EDIT_WIDGET_H
