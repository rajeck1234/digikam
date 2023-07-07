/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 28/08/2021
 * Description : a command line tool to detect image compression level
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2021-2022 by Phuoc Khanh Le <phuockhanhnk94 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// Qt includes

#include <QApplication>
#include <QDebug>

// Local includes

#include "imgqsorttest_shared.h"
#include "digikam_globals.h"
#include "dpluginloader.h"

using namespace Digikam;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "detectcompression - Parse image data to detect compression level";
        qDebug() << "Usage: <image file>";
        return -1;
    }

    QApplication app(argc, argv);

    DPluginLoader::instance()->init();

    QString path                = QString::fromUtf8(argv[1]);
    QFileInfoList list          = QFileInfoList() << QFileInfo(path);
    ImgQSortTest_ParseTestImagesDefautDetection(DETECTCOMPRESSION, list);

    return 0;
}
