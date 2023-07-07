/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-09-06
 * Description : a dialog to control camera capture.
 *
 * SPDX-FileCopyrightText: 2007-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAPTURE_DLG_H
#define DIGIKAM_CAPTURE_DLG_H

// Qt includes

#include <QCloseEvent>
#include <QDialog>

// Local includes

#include "digikam_export.h"

class QWidget;

namespace Digikam
{

class CameraController;

class CaptureDlg : public QDialog
{
    Q_OBJECT

public:

    CaptureDlg(QWidget* const parent,
               CameraController* const controller,
               const QString& cameraTitle);
    ~CaptureDlg()                   override;

protected:

    void closeEvent(QCloseEvent* e) override;

private Q_SLOTS:

    void slotPreview();
    void slotPreviewDone(const QImage&);
    void slotCapture();
    void slotCancel();
    void slotHelp();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_CAPTURE_DLG_H
