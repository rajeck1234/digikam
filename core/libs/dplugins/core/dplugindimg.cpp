/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2019-09-19
 * Description : digiKam plugin definition for DImg image loader.
 *
 * SPDX-FileCopyrightText: 2019-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "dplugindimg.h"

// Qt includes

#include <QMimeDatabase>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_version.h"
#include "digikam_debug.h"

namespace Digikam
{

DPluginDImg::DPluginDImg(QObject* const parent)
    : DPlugin(parent)
{
}

DPluginDImg::~DPluginDImg()
{
}

QMap<QString, QStringList> DPluginDImg::extraAboutData() const
{
    QMap<QString, QStringList> map;
    QMimeDatabase mimeDb;

    Q_FOREACH (const QString& ext, typeMimes().split(QLatin1Char(' ')))
    {
        if (!ext.isEmpty())
        {
            QMimeType mime = mimeDb.mimeTypeForFile(QString::fromLatin1("foo.%1").arg(ext));

            if (mime.name() == QLatin1String("application/octet-stream"))
            {
                map.insert(ext,
                           QStringList() << i18nc("@info: type of image", "%1 image", ext)
                                         << ((canRead(QFileInfo(QString::fromLatin1("foo.%1").arg(ext)), false) != 0) ?
                                             i18nc("@info: can read file format",     "yes") :
                                             i18nc("@info: cannot read file format",  "no"))
                                         << ((canWrite(ext) != 0) ?
                                             i18nc("@info: can write file format",    "yes") :
                                             i18nc("@info: cannot write file format", "no"))
                );
            }
            else
            {
                map.insert(ext,
                           QStringList() << mime.comment()
                                         << ((canRead(QFileInfo(QString::fromLatin1("foo.%1").arg(ext)), false) != 0) ?
                                             i18nc("@info: can read file format",     "yes") :
                                             i18nc("@info: cannot read file format",  "no"))
                                         << ((canWrite(ext) != 0) ?
                                             i18nc("@info: can write file format",    "yes") :
                                             i18nc("@info: cannot write file format", "no"))
                );
            }
        }
    }

    return map;
}

QStringList DPluginDImg::extraAboutDataRowTitles() const
{
    return QStringList() << i18nc("@title: DPlugin properties", "Extension")
                         << i18nc("@title: DPlugin properties", "Description")
                         << i18nc("@title: DPlugin properties", "Read")
                         << i18nc("@title: DPlugin properties", "Write");
}

QString DPluginDImg::extraAboutDataTitle() const
{
    return i18nc("@title", "Type-Mimes");
}

} // namespace Digikam
