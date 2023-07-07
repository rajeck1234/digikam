/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2017-09-24
 * Description : a media server to export collections through DLNA.
 *
 * SPDX-FileCopyrightText: 2017-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DMEDIA_SERVER_H
#define DIGIKAM_DMEDIA_SERVER_H

// Qt includes

#include <QObject>
#include <QString>
#include <QUrl>
#include <QMap>
#include <QList>

namespace DigikamGenericMediaServerPlugin
{

/// A kind of map of albums with urls contents to share with DLNA media server.
typedef QMap<QString, QList<QUrl> > MediaServerMap;

class DMediaServer : public QObject
{
    Q_OBJECT

public:

    explicit DMediaServer(QObject* const parent = nullptr);
    ~DMediaServer() override;

    /**
     * Initialize the internal server instance and return true if all is ready to host contents.
     * If port = 0, the server will select one automatically, else it will use the specified one.
     */
    bool init(int port = 0);

    /**
     * To share a list of albums with dedicated urls list for each one.
     */
    void addAlbumsOnServer(const MediaServerMap& map);

private:

    class Private;
    Private* const d;

private:

    Q_DISABLE_COPY(DMediaServer)
};

} // namespace DigikamGenericMediaServerPlugin

#endif // DIGIKAM_DMEDIA_SERVER_H
