/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * SPDX-FileCopyrightText: 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_RAJCE_NEW_ALBUM_DLG_H
#define DIGIKAM_RAJCE_NEW_ALBUM_DLG_H

// Local includes

#include "wsnewalbumdialog.h"

class QCheckBox;

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

class RajceNewAlbumDlg : public WSNewAlbumDialog
{
    Q_OBJECT

public:

    explicit RajceNewAlbumDlg(QWidget* const parent = nullptr);
    ~RajceNewAlbumDlg() override;

public:

    QString albumName()        const;
    QString albumDescription() const;
    bool    albumVisible()     const;

private:

    QCheckBox* m_albumVisible;
};

} // namespace DigikamGenericRajcePlugin

#endif // DIGIKAM_RAJCE_NEW_ALBUM_DLG_H
