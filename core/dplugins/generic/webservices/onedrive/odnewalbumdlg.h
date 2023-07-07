/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_OD_NEW_ALBUM_DLG_H
#define DIGIKAM_OD_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace DigikamGenericOneDrivePlugin
{

class ODFolder;

class ODNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit ODNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~ODNewAlbumDlg() override;

    void getFolderTitle(ODFolder& folder);
};

} // namespace DigikamGenericOneDrivePlugin

#endif // DIGIKAM_OD_NEW_ALBUM_DLG_H
