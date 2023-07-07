/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-07-07
 * Description : a tool to export images to Flickr web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "flickrmpform.h"

// Qt includes

#include <QFile>
#include <QApplication>
#include <QUrl>
#include <QMimeDatabase>
#include <QMimeType>
#include <QtGlobal>
#include <QTime>

// Local includes

#include "digikam_debug.h"
#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericFlickrPlugin
{

FlickrMPForm::FlickrMPForm()
    : m_boundary(QByteArray("----------") + WSToolUtils::randomString(42 + 13).toLatin1())
{
}

FlickrMPForm::~FlickrMPForm()
{
}

void FlickrMPForm::reset()
{
    m_buffer.resize(0);
}

void FlickrMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

bool FlickrMPForm::addPair(const QString& name, const QString& value, const QString& contentType)
{
    QByteArray str;
    QString content_length = QString::fromLatin1("%1").arg(value.length());

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toLatin1();
        str += "\"\r\n";
    }

    if (!contentType.isEmpty())
    {
        str += "Content-Type: " + QByteArray(contentType.toLatin1());
        str += "\r\n";
        str += "Mime-version: 1.0 ";
        str += "\r\n";
    }

    str += "Content-Length: ";
    str += content_length.toLatin1();
    str += "\r\n\r\n";
    str += value.toUtf8();

    m_buffer.append(str);
    m_buffer.append("\r\n");
    return true;
}

bool FlickrMPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();

    if (mime.isEmpty())
    {
        // if we ourselves can't determine the mime of the local file,
        // very unlikely the remote site will be able to identify it
        return false;
    }

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QByteArray imageData = imageFile.readAll();

    QByteArray str;
    QString file_size = QString::fromLatin1("%1").arg(imageFile.size());
    imageFile.close();

    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; name=\"";
    str += name.toLatin1();
    str += "\"; ";
    str += "filename=\"";
    str += QFile::encodeName(QUrl::fromLocalFile(path).fileName());
    str += "\"\r\n";
    str += "Content-Length: ";
    str += file_size.toLatin1();
    str += "\r\n";
    str += "Content-Type: ";
    str +=  mime.toLatin1();
    str += "\r\n\r\n";

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString FlickrMPForm::contentType() const
{
    return QLatin1String("multipart/form-data; boundary=") + QLatin1String(m_boundary);
}

QString FlickrMPForm::boundary() const
{
    return QLatin1String(m_boundary);
}

QByteArray FlickrMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericFlickrPlugin
