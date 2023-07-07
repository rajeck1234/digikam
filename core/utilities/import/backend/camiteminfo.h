/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2004-09-18
 * Description : camera item info container
 *
 * SPDX-FileCopyrightText: 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAMITEM_INFO_H
#define DIGIKAM_CAMITEM_INFO_H

// Qt includes

#include <QList>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QUrl>

// Local includes

#include "photoinfocontainer.h"
#include "videoinfocontainer.h"
#include "digikam_export.h"

class QDataStream;

namespace Digikam
{

class DIGIKAM_GUI_EXPORT CamItemInfo
{

public:

    enum DownloadStatus
    {
        DownloadUnknown  = -1,               ///< Download state is unknown
        DownloadedNo     = 0,                ///< Is not yet downloaded on computer
        DownloadedYes    = 1,                ///< Is already downloaded on computer
        DownloadFailed   = 2,                ///< Download is failed or have been aborted by user
        DownloadStarted  = 3,                ///< Download is under progress
        NewPicture       = 4                 ///< This is a new item from camera
    };

public:

    explicit CamItemInfo();
    ~CamItemInfo();

    /**
     * Return true if all member in this container are null.
     */
    bool isNull()                            const;

    /**
     * Return the local file system (mounted on computer) url to the camera file.
     */
    QUrl url()                               const;

    /**
     * Compare for camera information equality, not including variable values.
     */
    bool operator==(const CamItemInfo& info) const;

    /**
     * Compare for camera information un-equality, not including variable values.
     */
    bool operator!=(const CamItemInfo& info) const;

public:

    /// Static values taken from camera.
    qint64             size;                 ///< Camera file size in bytes.

    int                width;                ///< Image width in pixels
    int                height;               ///< Image height in pixels
    int                readPermissions;      ///< Read permission of camera file
    int                writePermissions;     ///< Write permission of camera file

    QString            name;                 ///< File name in camera file-system
    QString            folder;               ///< Folder path to access to file in camera
    QString            mime;                 ///< Type mime of camera file

    QDateTime          ctime;                ///< Created time stamp of camera file

    PhotoInfoContainer photoInfo;            ///< Photo Info from camera file (get from file metadata)

    /// Variable values depending of user actions.
    int                downloaded;           ///< Download status of camera file. See DownloadStatus enum for details
    QString            downloadName;         ///< Preview of the file-name to use during download from camera

    /// Unique image id
    qlonglong          id;

    int                rating;               ///< Pre-rating value of camera file.

    int                pickLabel;            ///< Pre-picklabel value of camera file.

    int                colorLabel;           ///< Pre-picklabel value of camera file.

    QList<int>         tagIds;               ///< Pre-tags ids of camera file.
    bool               previewPossible;
};

QDataStream& operator<<(QDataStream&, const CamItemInfo&);
QDataStream& operator>>(QDataStream&, CamItemInfo&);

typedef QList<CamItemInfo> CamItemInfoList;

//! qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const CamItemInfo& info);

} // namespace Digikam

#endif // DIGIKAM_CAMITEM_INFO_H
