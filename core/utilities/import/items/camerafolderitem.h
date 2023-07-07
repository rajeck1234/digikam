/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-01-23
 * Description : A widget to display a camera folder.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAMERA_FOLDER_ITEM_H
#define DIGIKAM_CAMERA_FOLDER_ITEM_H

// Qt includes

#include <QString>
#include <QPixmap>
#include <QTreeWidgetItem>
#include <QIcon>

namespace Digikam
{

class CameraFolderItem : public QTreeWidgetItem
{
public:

    CameraFolderItem(QTreeWidget* const parent,
                     const QString& name,
                     const QIcon &icon = QIcon::fromTheme(QLatin1String("folder")));

    CameraFolderItem(QTreeWidgetItem* const parent,
                     const QString& folderName,
                     const QString& folderPath,
                     const QIcon& icon = QIcon::fromTheme(QLatin1String("folder")));

    ~CameraFolderItem()               override;

    QString folderName()        const;
    QString folderPath()        const;
    bool    isVirtualFolder()   const;

    void    changeCount(int val);
    void    setCount(int val);
    int     count()             const;

private:

    class Private;
    Private* const d;

private:

    Q_DISABLE_COPY(CameraFolderItem)
};

} // namespace Digikam

#endif // DIGIKAM_CAMERA_FOLDER_ITEM_H
