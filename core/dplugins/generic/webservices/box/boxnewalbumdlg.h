/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Box web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_BOX_NEW_ALBUM_DLG_H
#define DIGIKAM_BOX_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace DigikamGenericBoxPlugin
{

class BOXFolder;

class BOXNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit BOXNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~BOXNewAlbumDlg() override;

    void getFolderTitle(BOXFolder& folder);
};

} // namespace DigikamGenericBoxPlugin

#endif // DIGIKAM_BOX_NEW_ALBUM_DLG_H
