/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-06-29
 * Description : a tool to export images to Twitter social network
 *
 * SPDX-FileCopyrightText: 2018 by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_TWITTER_NEW_ALBUM_DLG_H
#define DIGIKAM_TWITTER_NEW_ALBUM_DLG_H

// Qt includes

#include <QDialog>

// Local includes

#include "wsnewalbumdialog.h"

using namespace Digikam;

class QComboBox;

namespace DigikamGenericTwitterPlugin
{

class TwAlbum;

class TwNewAlbumDlg : public WSNewAlbumDialog
{
     Q_OBJECT

public:

     explicit TwNewAlbumDlg(QWidget* const parent, const QString& toolName);
     ~TwNewAlbumDlg() override;

     void getAlbumProperties(TwAlbum& album);

private:

     friend class TwWindow;
};

} // namespace DigikamGenericTwitterPlugin

#endif // DIGIKAM_TWITTER_NEW_ALBUM_DLG_H
