/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : slide show tool using preview of pictures.
 *
 * SPDX-FileCopyrightText: 2005-2023 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * SPDX-FileCopyrightText: 2019-2020 by Minh Nghia Duong <minhnghiaduong997 at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_SLIDE_SHOW_LOADER_H
#define DIGIKAM_SLIDE_SHOW_LOADER_H

// Qt includes

#include <QKeyEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QStackedWidget>

// Local includes

#include "digikam_export.h"
#include "dinfointerface.h"
#include "loadingdescription.h"
#include "slideshowsettings.h"
#include "dimg.h"

using namespace Digikam;

namespace DigikamGenericSlideShowPlugin
{

class SlideShowLoader : public QStackedWidget
{
    Q_OBJECT

public:

    enum SlideShowViewMode
    {
        ErrorView = 0,
        ImageView,
        VideoView,
        EndView
    };

public:

    explicit SlideShowLoader(SlideShowSettings* const settings);
    ~SlideShowLoader()                          override;

    void setCurrentItem(const QUrl& url);
    QUrl currentItem() const;

    void setShortCutPrefixes(const QMap<QString, QString>& prefixes);

Q_SIGNALS:

    void signalLastItemUrl(const QUrl&);

public Q_SLOTS:

    void slotRemoveImageFromList();
    void slotLoadNextItem();
    void slotLoadPrevItem();
    void slotPause();
    void slotPlay();

    void slotAssignRating(int);
    void slotAssignColorLabel(int);
    void slotAssignPickLabel(int);
    void slotToggleTag(int tag);

    void slotHandleShortcut(const QString& shortcut, int val);

protected:

    void mousePressEvent(QMouseEvent*)          override;
    void keyPressEvent(QKeyEvent*)              override;
    void wheelEvent(QWheelEvent*)               override;

private Q_SLOTS:

    void slotMouseMoveTimeOut();
    void slotImageLoaded(bool);
    void slotScreenSelected(int);
    void slotVideoLoaded(bool);
    void slotVideoFinished();

private:

    void setCurrentView(SlideShowViewMode);
    bool eventFilter(QObject* obj, QEvent* ev)  override;
    void preloadNextItem();
    void endOfSlide();
    void inhibitScreenSaver();
    void allowScreenSaver();
    void dispatchCurrentInfoChange(const QUrl& url);
    void makeCornerRectangles(const QRect& desktopRect, const QSize& size,
                              QRect* const topLeft, QRect* const topRight,
                              QRect* const topLeftLarger, QRect* const topRightLarger);

private:

    // Disable
    explicit SlideShowLoader(QWidget*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericSlideShowPlugin

#endif // DIGIKAM_SLIDE_SHOW_LOADER_H
