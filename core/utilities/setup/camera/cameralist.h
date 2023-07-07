/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : Cameras list container
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAMERA_LIST_H
#define DIGIKAM_CAMERA_LIST_H

// Qt includes

#include <QList>
#include <QObject>

// Local includes

#include "digikam_export.h"

class QString;
class QDateTime;
class QAction;

namespace Digikam
{

class CameraType;

class DIGIKAM_GUI_EXPORT CameraList : public QObject
{
    Q_OBJECT

public:

    CameraList(QObject* const parent, const QString& file);
    ~CameraList() override;

    bool load();
    bool save();
    void clear();

    void insert(CameraType* const ctype);
    void remove(CameraType* const ctype);

    CameraType*         autoDetect(bool& retry);
    CameraType*         find(const QString& title) const;
    QList<CameraType*>* cameraList()               const;

    bool changeCameraStartIndex(const QString& cameraTitle, int startIndex);

    static bool findConnectedCamera(int vendorId, int productId, QString& model, QString& port);

    static CameraList* defaultList();

Q_SIGNALS:

    void signalCameraAdded(CameraType*);
    void signalCameraRemoved(QAction*);

private:

    void insertPrivate(CameraType* const ctype);
    void removePrivate(CameraType* const ctype);

private:

    static CameraList* m_defaultList;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_CAMERA_LIST_H
