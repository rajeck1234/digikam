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

#ifndef DIGIKAM_OD_WINDOW_H
#define DIGIKAM_OD_WINDOW_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QPair>

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"

class QCloseEvent;
class QUrl;

using namespace Digikam;

namespace DigikamGenericOneDrivePlugin
{

class ODWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit ODWindow(DInfoInterface* const iface, QWidget* const parent);
    ~ODWindow() override;

    void reactivate();

    void setItemsList(const QList<QUrl>& urls);

private:

    void readSettings();
    void writeSettings();

    void uploadNextPhoto();

    void buttonStateChange(bool state);
    void closeEvent(QCloseEvent*) override;

private Q_SLOTS:

    void slotImageListChanged();
    void slotUserChangeRequest();
    void slotNewAlbumRequest();
    void slotReloadAlbumsRequest();
    void slotStartTransfer();

    void slotBusy(bool);
    void slotSignalLinkingFailed();
    void slotSignalLinkingSucceeded();
    void slotSetUserName(const QString& msg);
    void slotListAlbumsFailed(const QString& msg);
    void slotListAlbumsDone(const QList<QPair<QString, QString> >& list);
    void slotCreateFolderFailed(const QString& msg);
    void slotCreateFolderSucceeded();
    void slotAddPhotoFailed(const QString& msg);
    void slotAddPhotoSucceeded();
    void slotTransferCancel();

    void slotFinished();

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericOneDrivePlugin

#endif // DIGIKAM_OD_WINDOW_H
