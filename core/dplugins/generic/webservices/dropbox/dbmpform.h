/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-18
 * Description : a tool to export images to Dropbox web service
 *
 * SPDX-FileCopyrightText: 2013      by Pankaj Kumar <me at panks dot me>
 * SPDX-FileCopyrightText: 2013-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DB_MPFORM_H
#define DIGIKAM_DB_MPFORM_H

// Qt includes

#include <QByteArray>

namespace DigikamGenericDropBoxPlugin
{

class DBMPForm
{

public:

    explicit DBMPForm();
    ~DBMPForm();

    bool addFile(const QString& imgPath);
    QByteArray formData() const;

private:

    QByteArray m_buffer;
};

} // namespace DigikamGenericDropBoxPlugin

#endif // DIGIKAM_DB_MPFORM_H
