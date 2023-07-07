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

#ifndef DIGIKAM_FLICKR_MPFORM_H
#define DIGIKAM_FLICKR_MPFORM_H

// Qt includes

#include <QByteArray>
#include <QString>

namespace DigikamGenericFlickrPlugin
{

class FlickrMPForm
{

public:

    explicit FlickrMPForm();
    ~FlickrMPForm();

    void finish();
    void reset();

    bool addPair(const QString& name, const QString& value, const QString& type);
    bool addFile(const QString& name, const QString& path);

    QString    contentType() const;
    QByteArray formData()    const;
    QString    boundary()    const;

private:

    QByteArray m_buffer;
    QByteArray m_boundary;
};

} // namespace DigikamGenericFlickrPlugin

#endif // DIGIKAM_FLICKR_MPFORM_H
