/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2021-07-21
 * Description : MJPEG Stream Server configuration dialog
 *
 * SPDX-FileCopyrightText: 2021-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_MJPEG_STREAM_DLG_H
#define DIGIKAM_MJPEG_STREAM_DLG_H

// Qt includes

#include <QUrl>

// Local includes

#include "dinfointerface.h"
#include "dplugindialog.h"

using namespace Digikam;

namespace DigikamGenericMjpegStreamPlugin
{

class MjpegStreamDlg : public DPluginDialog
{
    Q_OBJECT

//@{
// Base methods - mjgpegstreamdlg.cpp

public:

    explicit MjpegStreamDlg(QObject* const parent, DInfoInterface* const iface = nullptr);
    ~MjpegStreamDlg()               override;

private Q_SLOTS:

    void accept()                   override;
    void slotSelectionChanged();
    void slotOpenPreview();

//@}

//@{
// Settings methods - mjpegstreamdlg_settings.cpp

private:


    void readSettings();
    void saveSettings();

private Q_SLOTS:

    void slotSettingsChanged();

//@}

//@{
// Server methods - mjpegstreamdlg_server.cpp

private:

    void updateServerStatus();
    bool setMjpegServerContents();
    bool startMjpegServer();
    void setupOSDView();

private Q_SLOTS:

    void slotToggleMjpegServer();

//@}

//@{
// Views methods - mjpegstreamdlg_views.cpp

private:

    QWidget* setupItemsView();
    void setupServerView();
    void setupStreamView();
    void setupTransitionView();
    void setupEffectView();

private Q_SLOTS:

    void slotSetUnchecked(int);

//@}

private:

    // Disable
    explicit MjpegStreamDlg(QWidget*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericMjpegStreamPlugin

#endif // DIGIKAM_MJPEG_STREAM_DLG_H
