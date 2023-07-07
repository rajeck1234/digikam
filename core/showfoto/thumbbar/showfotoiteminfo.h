/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-07-1
 * Description : Showfoto item info container
 *
 * SPDX-FileCopyrightText: 2013 by Mohamed_Anwer <m_dot_anwer at gmx dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef SHOW_FOTO_ITEM_INFO_H
#define SHOW_FOTO_ITEM_INFO_H

// Qt includes

#include <QList>
#include <QByteArray>
#include <QDebug>
#include <QUrl>
#include <QFileInfo>

// Local includes

#include "photoinfocontainer.h"
#include "videoinfocontainer.h"

using namespace Digikam;

class QDataStream;

namespace ShowFoto
{

class ShowfotoItemInfo
{
public:

    explicit ShowfotoItemInfo();
    ~ShowfotoItemInfo();

    /**
     * Return true if all member in this container are null.
     */
    bool isNull() const;

    /**
     * Compare for information equality and un-equality, not including variable values.
     */
    bool operator==(const ShowfotoItemInfo& info) const;
    bool operator!=(const ShowfotoItemInfo& info) const;

    static ShowfotoItemInfo itemInfoFromFile(const QFileInfo& inf);

public:

    /// Static values.
    qint64             size;                 ///< file size in bytes.
    QUrl               url;                  ///< file Url

    QString            name;                 ///< File name in file-system
    QString            folder;               ///< Folder path to access to file
    QString            mime;                 ///< Type mime of file

    /// Unique image id
    qlonglong          id;

    PhotoInfoContainer photoInfo;

    QDateTime          dtime;                ///< creation time on disk
    QDateTime          ctime;                ///< camera date stamp
    int                width;                ///< Image width in pixels
    int                height;               ///< Image height in pixels
};

QDataStream& operator<<(QDataStream&, const ShowfotoItemInfo&);
QDataStream& operator>>(QDataStream&, ShowfotoItemInfo&);

typedef QList<ShowfotoItemInfo> ShowfotoItemInfoList;

//! qDebug() stream operator. Writes property @a info to the debug output in a nicely formatted way.
QDebug operator<<(QDebug dbg, const ShowfotoItemInfo& info);

} // namespace Showfoto

#endif // SHOW_FOTO_ITEM_INFO_H
