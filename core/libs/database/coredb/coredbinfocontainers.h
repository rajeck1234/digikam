/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-04
 * Description : Container classes holding user presentable information
 *
 * SPDX-FileCopyrightText: 2008-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CORE_DB_INFO_CONTAINERS_H
#define DIGIKAM_CORE_DB_INFO_CONTAINERS_H

// Qt includes

#include <QString>
#include <QDateTime>

namespace Digikam
{

class ImageCommonContainer
{
public:

    explicit ImageCommonContainer()
      : fileSize  (0),
        rating    (-1),
        width     (0),
        height    (0),
        colorDepth(0)
    {
    }

    QString   fileName;
    QDateTime fileModificationDate;
    qint64    fileSize;

    int       rating;
    QDateTime creationDate;
    QDateTime digitizationDate;
    QString   orientation;
    int       width;
    int       height;
    QString   format;
    int       colorDepth; // bits per channel, 8/16
    QString   colorModel;
};

// ------------------------------------------------------------

class ImageMetadataContainer
{
public:

    explicit ImageMetadataContainer()
      : allFieldsNull(true)
    {
    }

    bool allFieldsNull;

    QString make;
    QString model;
    QString lens;
    QString aperture;
    QString focalLength;
    QString focalLength35;
    QString exposureTime;
    QString exposureProgram;
    QString exposureMode;
    QString sensitivity;
    QString flashMode;
    QString whiteBalance;
    QString whiteBalanceColorTemperature;
    QString meteringMode;
    QString subjectDistance;
    QString subjectDistanceCategory;
};

// ------------------------------------------------------------

class VideoMetadataContainer
{
public:

    explicit VideoMetadataContainer()
      : allFieldsNull(true)
    {
    }

    bool    allFieldsNull;

    QString aspectRatio;
    QString audioBitRate;
    QString audioChannelType;
    QString audioCodec;
    QString duration;
    QString frameRate;
    QString videoCodec;
};

} // namespace Digikam

#endif // DIGIKAM_CORE_DB_INFO_CONTAINERS_H
