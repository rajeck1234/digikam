/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-11
 * Description : a tool to show image using an OpenGL interface.
 *
 * SPDX-FileCopyrightText: 2007-2008 by Markus Leuthold <kusi at forum dot titlis dot org>
 * SPDX-FileCopyrightText: 2008-2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#ifndef DIGIKAM_GLVIEWER_PLUGIN_GLVIEWER_WIDGET_H
#define DIGIKAM_GLVIEWER_PLUGIN_GLVIEWER_WIDGET_H

// Qt includes

#include <QCursor>
#include <QDir>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QWheelEvent>
#include <QUrl>

// Local includes

#include "glviewertexture.h"
#include "glviewerglobal.h"
#include "dplugin.h"
#include "dinfointerface.h"

using namespace Digikam;

namespace DigikamGenericGLViewerPlugin
{

class GLViewerWidget : public QOpenGLWidget
{
    Q_OBJECT

public:

    explicit GLViewerWidget(DPlugin* const plugin,
                            DInfoInterface* const iface);
    ~GLViewerWidget() override;

    void prevImage();
    void nextImage();
    bool listOfFilesIsEmpty()                  const;

    OGLstate getOGLstate()                     const;

private:

    void initializeGL()                              override;
    void resizeGL(int w, int h)                      override;
    void paintGL()                                   override;

    bool isReallyFullScreen()                  const;

    void keyPressEvent(QKeyEvent* e)                 override;
    void keyReleaseEvent(QKeyEvent* e)               override;
    void wheelEvent(QWheelEvent* e)                  override;
    void mouseMoveEvent(QMouseEvent* e)              override;
    void mousePressEvent(QMouseEvent* e)             override;
    void mouseDoubleClickEvent(QMouseEvent* e)       override;
    void mouseReleaseEvent(QMouseEvent* e)           override;

    GLViewerTexture* loadImage(int file_index) const;
    void drawImage(GLViewerTexture* const tex);
    void downloadTexture(GLViewerTexture* const tex);

    void zoom(int mdelta,
              const QPoint& pos,
              float factor);

private Q_SLOTS:

    void slotTimeoutMouseMove();

private:

    // Disable
    explicit GLViewerWidget(QWidget*) = delete;

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericGLViewerPlugin

#endif // DIGIKAM_GLVIEWER_PLUGIN_GLVIEWER_WIDGET_H
