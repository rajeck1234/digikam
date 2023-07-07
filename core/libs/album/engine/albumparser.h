/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-01-24
 * Description : album parser progress indicator
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ALBUM_PARSER_H
#define DIGIKAM_ALBUM_PARSER_H

// Local includes

#include "iteminfo.h"
#include "progressmanager.h"

namespace Digikam
{
class Album;

class DIGIKAM_GUI_EXPORT AlbumParser : public ProgressItem
{
    Q_OBJECT

public:

    /**
     * Constructor to work on image list
     */
    explicit AlbumParser(const ItemInfoList& infoList);

    /**
     * Constructor to work on recursive mode from album
     */
    explicit AlbumParser(Album* const album);

    ~AlbumParser() override;

    void run();

Q_SIGNALS:

    void signalComplete(const QList<QUrl>&);

private Q_SLOTS:

    void slotRun();
    void slotCancel();
    void slotParseItemInfoList(const ItemInfoList& list);

private:

    // Disable
    AlbumParser(QObject*) = delete;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_ALBUM_PARSER_H
