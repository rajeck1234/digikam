/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-12-28
 * Description : a tool to export items to Facebook web service
 *
 * SPDX-FileCopyrightText: 2005-2008 by Vardhman Jain <vardhman at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2008-2009 by Luka Renko <lure at kubuntu dot org>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "fbmpform.h"

// Qt includes

#include <QFile>
#include <QMimeDatabase>
#include <QMimeType>
#include <QUrl>
#include <QString>

// Local includes

#include "digikam_debug.h"
#include "wstoolutils.h"

using namespace Digikam;

namespace DigikamGenericFaceBookPlugin
{

FbMPForm::FbMPForm()
    : m_boundary(WSToolUtils::randomString(42 + 13).toLatin1())
{
    reset();
}

FbMPForm::~FbMPForm()
{
}

void FbMPForm::reset()
{
    m_buffer.resize(0);
    QByteArray str(contentType().toLatin1());
    str += "\r\n";
    str += "MIME-version: 1.0";
    str += "\r\n\r\n";
    m_buffer.append(str);
}

void FbMPForm::finish()
{
    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "--";
    m_buffer.append(str);
}

void FbMPForm::addPair(const QString& name, const QString& value)
{
    QByteArray str;
    QString content_length = QString::number(value.length());

    str += "--";
    str += m_boundary;
    str += "\r\n";

    if (!name.isEmpty())
    {
        str += "Content-Disposition: form-data; name=\"";
        str += name.toLatin1();
        str += "\"\r\n";
    }

    str += "\r\n";
    str += value.toUtf8();
    str += "\r\n";

    m_buffer.append(str);
}

bool FbMPForm::addFile(const QString& name, const QString& path)
{
    QMimeDatabase db;
    QMimeType ptr = db.mimeTypeForUrl(QUrl::fromLocalFile(path));
    QString mime  = ptr.name();

    if (mime.isEmpty())
        return false;

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << "mime = " << mime.toLatin1();

    QFile imageFile(path);

    if (!imageFile.open(QIODevice::ReadOnly))
        return false;

    QByteArray imageData = imageFile.readAll();

    //QString file_size = QString::number(imageFile.size());
    imageFile.close();

    QByteArray str;
    str += "--";
    str += m_boundary;
    str += "\r\n";
    str += "Content-Disposition: form-data; filename=\"";
    str += QFile::encodeName(name);
    str += "\"\r\n";
    //str += "Content-Length: ";
    //str += file_size.toAscii();
    //str += "\r\n";
    str += "Content-Type: ";
    str += mime.toLatin1();
    str += "\r\n\r\n";

    qCDebug(DIGIKAM_WEBSERVICES_LOG) << str;

    m_buffer.append(str);
    m_buffer.append(imageData);
    m_buffer.append("\r\n");

    return true;
}

QString FbMPForm::contentType() const
{
    return QLatin1String("multipart/form-data; boundary=") + QLatin1String(m_boundary);
}

QString FbMPForm::boundary() const
{
    return QLatin1String(m_boundary);
}

QByteArray FbMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericFaceBookPlugin
