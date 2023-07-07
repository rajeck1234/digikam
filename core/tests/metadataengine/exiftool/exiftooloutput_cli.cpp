/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2013-11-28
 * Description : a command line tool to test ExifTool output without Exiv2 translation.
 *
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QCoreApplication>
#include <QVariant>
#include <QObject>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "exiftoolparser.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);

    if (argc != 2)
    {
        qCDebug(DIGIKAM_TESTS_LOG) << "exiftooloutput_cli - CLI tool to print ExifTool output without Exiv2 translation";
        qCDebug(DIGIKAM_TESTS_LOG) << "Usage: <image>";
        return -1;
    }

    // Create ExifTool parser instance.

    ExifToolParser* const parser = new ExifToolParser(qApp);

    // Read metadata from the file. Start ExifToolParser

    if (!parser->load(QString::fromUtf8(argv[1])))
    {
        return -1;
    }

    QString path                        = parser->currentPath();
    ExifToolParser::ExifToolData parsed = parser->currentData();

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << "Source File:" << path;

    // Print returned and sorted tags.

    QString     output;
    QTextStream stream(&output);
    QStringList tagsLst;

    const int section1 = -60;   // ExifTool Tag name simplified
    const int section2 = -30;   // Tag value as string.
    QString sep        = QString().fill(QLatin1Char('-'), qAbs(section1 + section2) + 4);

    // Header

    stream << sep
           << QT_ENDL
           << QString::fromLatin1("%1").arg(QLatin1String("ExifTool::group0.name"), section1) << " | "
           << QString::fromLatin1("%1").arg(QLatin1String("String Value"),          section2)
           << QT_ENDL
           << sep
           << QT_ENDL;

    for (ExifToolParser::ExifToolData::const_iterator it = parsed.constBegin() ;
         it != parsed.constEnd() ; ++it)
    {
        QString tagNameExifTool = it.key();
        QString tagType         = it.value()[1].toString();
        QString data            = it.value()[0].toString();

        if (data.size() > -section2)
        {
            data = data.left(-section2 - 3) + QLatin1String("...");
        }

        tagsLst
                << QString::fromLatin1("%1 | %2")
                .arg(tagNameExifTool, section1)
                .arg(data,            section2)
               ;
    }

    tagsLst.sort();

    Q_FOREACH (const QString& tag, tagsLst)
    {
        stream << tag << QT_ENDL;
    }

    stream << sep << QT_ENDL;

    qCDebug(DIGIKAM_TESTS_LOG).noquote() << output;

    return 0;
}
