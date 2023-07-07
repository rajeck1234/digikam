/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : a dialog to create a new remote album for export tools
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2015      by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_FLICKR_NEW_ALBUM_DLG_H
#define DIGIKAM_FLICKR_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

class FPhotoSet;

class FlickrNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit FlickrNewAlbumDlg(QWidget* const parent, const QString& toolName);
    ~FlickrNewAlbumDlg() override;

    void getFolderProperties(FPhotoSet& folder);
};

} // namespace DigikamGenericFlickrPlugin

#endif // DIGIKAM_FLICKR_NEW_ALBUM_DLG_H
