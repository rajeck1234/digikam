/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : painter class to draw calendar.
 *
 * SPDX-FileCopyrightText: 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * SPDX-FileCopyrightText: 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * SPDX-FileCopyrightText: 2012      by Angelo Naselli <anaselli at linux dot it>
 * SPDX-FileCopyrightText: 2012-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_CAL_PAINTER_H
#define DIGIKAM_CAL_PAINTER_H

// Qt includes

#include <QObject>
#include <QPainter>

class QPaintDevice;

namespace DigikamGenericCalendarPlugin
{

class CalPainter : public QObject,      // clazy:exclude=ctor-missing-parent-argument
                   public QPainter
{
    Q_OBJECT

public:

    explicit CalPainter(QPaintDevice* const pDevice);
    ~CalPainter() override;

    void setImage(const QUrl& imagePath);
    void paint(int month);

Q_SIGNALS:

    void signalTotal(int total);
    void signalProgress(int value);
    void signalFinished();

public Q_SLOTS:

    void cancel();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_CAL_PAINTER_H
