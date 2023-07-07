/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-06-16
 * Description : a dialog to select a target album to download
 *               pictures from camera
 *
 * SPDX-FileCopyrightText: 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ALBUM_SELECT_DIALOG_H
#define DIGIKAM_ALBUM_SELECT_DIALOG_H

// Qt includes

#include <QString>
#include <QDialog>

// Local includes

#include "searchtextbar.h"

namespace Digikam
{

class PAlbum;

class AlbumSelectDialog : public QDialog
{
    Q_OBJECT

public:

    explicit AlbumSelectDialog(QWidget* const parent, PAlbum* const albumToSelect, const QString& header=QString());
    ~AlbumSelectDialog() override;

    static PAlbum* selectAlbum(QWidget* const parent, PAlbum* const albumToSelect, const QString& header=QString());

private Q_SLOTS:

    void slotSelectionChanged();
    void slotHelp();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ALBUM_SELECT_DIALOG_H
