/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Pinterest web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_P_NEW_ALBUM_DLG_H
#define DIGIKAM_P_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace DigikamGenericPinterestPlugin
{

class PFolder;

class PNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit PNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~PNewAlbumDlg() override;

    void getFolderTitle(PFolder& folder);
};

} // namespace DigikamGenericPinterestPlugin

#endif // DIGIKAM_P_NEW_ALBUM_DLG_H
