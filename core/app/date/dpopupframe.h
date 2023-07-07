/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Frame with popup menu behavior.
 *
 * SPDX-FileCopyrightText: 2011-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 1997      by Tim D. Gilman <tdgilman at best dot org>
 * SPDX-FileCopyrightText: 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * SPDX-FileCopyrightText: 2007      by John Layt <john at layt dot net>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_DPOP_UP_FRAME_H
#define DIGIKAM_DPOP_UP_FRAME_H

// Qt includes

#include <QFrame>

namespace Digikam
{

class DPopupFrame : public QFrame
{
    Q_OBJECT

public:

    /**
     * The constructor. Creates a dialog without buttons.
     */
    explicit DPopupFrame(QWidget* const parent = nullptr);

    /**
     * The destructor
     */
    ~DPopupFrame() override;

    /**
     * Set the main widget. You cannot set the main widget from the constructor,
     * since it must be a child of the frame itselfes.
     * Be careful: the size is set to the main widgets size. It is up to you to
     * set the main widgets correct size before setting it as the main
     * widget.
     */
    void setMainWidget(QWidget* const m);

    /**
     * The resize event. Simply resizes the main widget to the whole
     * widgets client size.
     */
    void resizeEvent(QResizeEvent* e) override;

    /**
     * Open the popup window at position pos.
     */
    void popup(const QPoint& p);

    /**
     * Execute the popup window.
     */
    int exec(const QPoint& p);

    /**
     * Execute the popup window.
     */
    int exec(int x, int y);

Q_SIGNALS:

    void leaveModality();

protected:

    /**
     * Catch key press events.
     */
    void keyPressEvent(QKeyEvent* e) override;

    /**
     * Catch hide events.
     */
    void hideEvent(QHideEvent* e)    override;

public Q_SLOTS:

    /**
     * Close the popup window. This is called from the main widget, usually.
     * @p r is the result returned from exec().
     */
    void close(int r);

private:

    class Private;
    Private* const d;

    friend class Private;

    Q_DISABLE_COPY(DPopupFrame)
};

} // namespace Digikam

#endif // DIGIKAM_DPOP_UP_FRAME_H
