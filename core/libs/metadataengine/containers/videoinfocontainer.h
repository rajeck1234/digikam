/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2006-04-21
 * Description : video information container
 *
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_VIDEO_INFO_CONTAINER_H
#define DIGIKAM_VIDEO_INFO_CONTAINER_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QDebug>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT VideoInfoContainer
{

public:

    explicit VideoInfoContainer();
    ~VideoInfoContainer();

    bool isEmpty()                                  const;
    bool isNull()                                   const;

    bool operator==(const VideoInfoContainer& t)    const;

public:

    QString aspectRatio;
    QString duration;
    QString frameRate;
    QString videoCodec;
    QString audioBitRate;
    QString audioChannelType;
    QString audioCodec;
};

DIGIKAM_EXPORT QDataStream& operator<<(QDataStream& ds, const VideoInfoContainer& info);
DIGIKAM_EXPORT QDataStream& operator>>(QDataStream& ds, VideoInfoContainer& info);

//! qDebug() stream operator. Writes property @a t to the debug output in a nicely formatted way.
DIGIKAM_EXPORT QDebug operator<<(QDebug dbg, const VideoInfoContainer& t);

} // namespace Digikam

#endif // DIGIKAM_VIDEO_INFO_CONTAINER_H
