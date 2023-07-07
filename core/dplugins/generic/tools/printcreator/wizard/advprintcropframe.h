/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2002-30-09
 * Description : a tool to print images
 *
 * SPDX-FileCopyrightText: 2002-2003 by Todd Shoemaker <todd at theshoemakers dot net>
 * SPDX-FileCopyrightText: 2007-2012 by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2006-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_ADV_PRINT_CROP_FRAME_H
#define DIGIKAM_ADV_PRINT_CROP_FRAME_H

// Qt includes

#include <QWidget>

class QResizeEvent;

namespace DigikamGenericPrintCreatorPlugin
{

class AdvPrintPhoto;

class AdvPrintCropFrame : public QWidget
{
    Q_OBJECT

public:

    explicit AdvPrintCropFrame(QWidget* const parent);
    ~AdvPrintCropFrame()                        override;

    void   init(AdvPrintPhoto* const photo,
                int  woutlay,
                int  houtlay,
                bool autoRotate,
                bool paint);

    void   setColor(const QColor&);
    QColor color() const;

    void   drawCropRectangle(bool draw = true);

protected:

    void paintEvent(QPaintEvent*)               override;
    void mousePressEvent(QMouseEvent*)          override;
    void mouseReleaseEvent(QMouseEvent*)        override;
    void mouseMoveEvent(QMouseEvent*)           override;
    void keyReleaseEvent(QKeyEvent*)            override;

private:

    QRect screenToPhotoRect(const QRect&) const;
    QRect photoToScreenRect(const QRect&) const;
    void  resizeEvent(QResizeEvent*)            override;
    void  updateImage();

private:

    class Private;
    Private* const d;
};

} // Namespace Digikam

#endif // DIGIKAM_ADV_PRINT_CROP_FRAME_H
