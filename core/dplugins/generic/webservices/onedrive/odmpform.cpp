/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2018-05-20
 * Description : a tool to export images to Onedrive web service
 *
 * SPDX-FileCopyrightText: 2018      by Tarek Talaat <tarektalaat93 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "odmpform.h"

// Qt includes

#include <QFile>

// Local includes

#include "digikam_debug.h"

namespace DigikamGenericOneDrivePlugin
{

ODMPForm::ODMPForm()
{
}

ODMPForm::~ODMPForm()
{
}

bool ODMPForm::addFile(const QString& imgPath)
{
    QFile file(imgPath);

    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    m_buffer = file.readAll();
    file.close();

    return true;
}

QByteArray ODMPForm::formData() const
{
    return m_buffer;
}

} // namespace DigikamGenericOneDrivePlugin
