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

#include "glviewerwidget.h"

// Qt includes

#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QUrl>
#include <QList>
#include <QIcon>
#include <QPointer>
#include <QMimeType>
#include <QMimeDatabase>
#include <QStandardPaths>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "glviewertimer.h"
#include "glviewertexture.h"
#include "dpluginaboutdlg.h"
#include "digikam_debug.h"

// OpenGL headers is not included automatically with ARM targets

#ifdef Q_PROCESSOR_ARM
#   include <GL/gl.h>
#endif

#ifndef GL_TEXTURE_RECTANGLE_ARB
#   define GL_TEXTURE_RECTANGLE_ARB   0x84F5
#endif

#ifndef GL_TEXTURE_RECTANGLE_NV
#   define GL_TEXTURE_RECTANGLE_NV    0x84F5
#endif

namespace DigikamGenericGLViewerPlugin
{

class Q_DECL_HIDDEN GLViewerWidget::Private
{
public:

    struct Cache
    {
        int              file_index;
        GLViewerTexture* texture;
    };

    enum WheelAction
    {
        zoomImage,
        changeImage
    };

public:

    explicit Private()
      :
        file_idx                (0),                    ///< index of picture to be displayed

        texture                 (nullptr),
        ratio_view_y            (0.0F),
        ratio_view_x            (0.0F),
        delta                   (0.0F),
        vertex_height           (0.0F),
        vertex_width            (0.0F),
        vertex_left             (0.0F),
        vertex_top              (0.0F),
        vertex_right            (0.0F),
        vertex_bottom           (0.0F),
        wheelAction             (zoomImage),
        firstImage              (true),

        /**
         * while zooming is performed, the image is downsampled to d->zoomsize. This seems to
         * be the optimal way for a PentiumM 1.4G, Nvidia FX5200. For a faster setup, this might
         * not be necessary anymore
         */
        zoomsize                (QSize(1024, 768)),

        /// load cursors for zooming and panning
        moveCursor              (QCursor(Qt::PointingHandCursor)),
        zoomCursor              (QCursor(QIcon::fromTheme(QLatin1String("zoom-in")).pixmap(64))),

        /// define zoomfactors for one zoom step
        zoomfactor_scrollwheel  (1.1F),
        zoomfactor_mousemove    (1.03F),
        zoomfactor_keyboard     (1.05F),

        /// get path of nullImage in case QImage can't load the image
        nullImage               (QIcon::fromTheme(QLatin1String("image-jpeg")).pixmap(256)),

        iface                   (nullptr),
        plugin                  (nullptr)
    {
        for (int i = 0 ; i < CACHESIZE ; ++i)
        {
            cache[i].file_index = 0;
            cache[i].texture    = nullptr;
        }
    }

    QStringList      files;
    unsigned int     file_idx;
    Cache            cache[CACHESIZE];
    GLViewerTexture* texture;

    float            ratio_view_y;
    float            ratio_view_x;
    float            delta;
    float            vertex_height;
    float            vertex_width;
    float            vertex_left;
    float            vertex_top;
    float            vertex_right;
    float            vertex_bottom;

    QPoint           startdrag;
    QPoint           previous_pos;
    WheelAction      wheelAction;
    bool             firstImage;
    QSize            zoomsize;
    QTimer           timerMouseMove;
    QCursor          moveCursor;
    QCursor          zoomCursor;
    float            zoomfactor_scrollwheel;
    float            zoomfactor_mousemove;
    float            zoomfactor_keyboard;
    QPixmap          nullImage;
    QSize            screenSize;

    DInfoInterface*  iface;
    DPlugin*         plugin;
};

GLViewerWidget::GLViewerWidget(DPlugin* const plugin, DInfoInterface* const iface)
    : QOpenGLWidget(),
      d            (new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);

    d->plugin       = plugin;
    d->iface        = iface;

    // Determine screen size for isReallyFullScreen

    QScreen* screen = qApp->primaryScreen();

    if (QWidget* const widget = qApp->activeWindow())
    {
        if (QWindow* const window = widget->windowHandle())
        {
            screen = window->screen();
        }
    }

    d->screenSize         = screen->size();

    QList<QUrl> myfiles;                                            // pics which are displayed in imageviewer
    QList<QUrl> selection = d->iface->currentSelectedItems();
    QString selectedImage;                                          // selected pic in hostapp

    int foundNumber       = 0;

    if      (selection.count() == 0)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "no image selected, load entire album";
        myfiles = d->iface->currentAlbumItems();
    }
    else if (selection.count() == 1)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "one image selected, load entire album and start with selected image";
        selectedImage = selection.first().toLocalFile();
        myfiles       = d->iface->currentAlbumItems();
    }
    else if (selection.count() > 1)
    {
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "load" << selection.count() << "selected images";
        myfiles = selection;
    }

    // populate QStringList::d->files

    for (QList<QUrl>::Iterator it = myfiles.begin() ; it != myfiles.end() ; ++it)
    {
        // find selected image in album in order to determine the first displayed image
        // in case one image was selected and the entire album was loaded

        QString s = it->toLocalFile();

        if (s == selectedImage)
        {
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "selected img" << selectedImage << "has idx=" << foundNumber;
            d->file_idx = foundNumber;
        }

        // only add images to d->files

        QString mimeTypeName = QMimeDatabase().mimeTypeForUrl(QUrl::fromLocalFile(s)).name();
        bool isImage         = mimeTypeName.contains(QString::fromLatin1("image"), Qt::CaseInsensitive);

        if (isImage)
        {
            d->files.append(s);
            foundNumber++;      // counter for searching the start image in case one image is selected
            qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << s << "type=" << mimeTypeName;
        }
    }

    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << d->files.count() << "images loaded";

    if (d->files.isEmpty())
    {
        return;
    }

    showFullScreen(); // krazy:exclude=qmethods

    // let the cursor disappear after 2sec of inactivity

    connect(&d->timerMouseMove, SIGNAL(timeout()),
            this, SLOT(slotTimeoutMouseMove()));

    d->timerMouseMove.start(2000);
    setMouseTracking(true);

    // other initialisations

    d->wheelAction = GLViewerWidget::Private::changeImage;
}

GLViewerWidget::~GLViewerWidget()
{
    for (int i = 0 ; i < CACHESIZE ; ++i)
    {
        d->cache[i].file_index = EMPTY;
        delete d->cache[i].texture;
    }

    delete d;
}

/**
 * \todo blending
 */
void GLViewerWidget::initializeGL()
{
    glEnable(GL_TEXTURE_RECTANGLE_ARB);

    // Clear The Background Color
    glClearColor(0.0, 0.0, 0.0, 1.0f);

    // Turn Blending On
    glEnable(GL_BLEND);

    // Blending Function For Translucency Based On Source Alpha Value
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable perspective vision
    glClearDepth(1.0f);

    // initialize cache
    for (int i = 0 ; i < CACHESIZE ; ++i)
    {
        d->cache[i].file_index = EMPTY;
        d->cache[i].texture    = new GLViewerTexture(d->iface);
    }
}

bool GLViewerWidget::listOfFilesIsEmpty() const
{
    return d->files.isEmpty();
}

void GLViewerWidget::paintGL()
{
    // this test has to be performed here since QWidget::width() is only updated now
/*
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "enter paintGL: isReallyFullscreen=" << isReallyFullScreen();
*/
    // prepare 1st image

    if (d->firstImage && isReallyFullScreen())
    {
/*
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "first image";
*/
        d->texture = loadImage(d->file_idx);
        d->texture->reset();
        downloadTexture(d->texture);
/*
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "width=" << width();
*/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        drawImage(d->texture);

        // trigger a redraw NOW. the user wants to see a picture as soon as possible
        // only load the second image after the first is displayed

        glFlush();

        // preload the 2nd image

        if (d->firstImage)
        {
            if (d->file_idx < (unsigned int)(d->files.count() - 1))
            {
                loadImage(d->file_idx+1);
            }

            d->firstImage = false;
        }
    }

    if (!d->firstImage)
    {
/*
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "width=" << width();
*/
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, -5.0f);
        drawImage(d->texture);
    }
/*
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "exit paintGL";
*/
}

void GLViewerWidget::resizeGL(int w, int h)
{
/*
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "resizeGL,w=" << w;
*/
    glViewport(0, 0, (GLint)w, (GLint)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    if (h > w)
    {
        d->ratio_view_x = 1.0;
        d->ratio_view_y = h / float(w);
    }
    else
    {
        d->ratio_view_x = w / float(h);
        d->ratio_view_y = 1.0;
    }

    glFrustum(-d->ratio_view_x, d->ratio_view_x, -d->ratio_view_y, d->ratio_view_y, 5, 5000.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (d->texture == nullptr)
    {
        return;
    }

    if (d->firstImage)
    {
        d->texture->setViewport(w, h);
    }
}

/**
 * render the image
 */
void GLViewerWidget::drawImage(GLViewerTexture* const tex)
{
/*
    qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "enter drawImage: target=" << d->texture->textureId()
                                         << "dim=" << d->texture->height() << d->texture->width();
*/
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex->textureId());
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex3f(tex->vertex_left(), tex->vertex_bottom(), 0);

    glTexCoord2f(tex->width(), 0);
    glVertex3f(d->texture->vertex_right(), tex->vertex_bottom(), 0);

    glTexCoord2f(tex->width(), tex->height());
    glVertex3f(tex->vertex_right(), tex->vertex_top(), 0);

    glTexCoord2f(0, tex->height());
    glVertex3f(tex->vertex_left(), tex->vertex_top(), 0);
    glEnd();
}

/**
 * Handle all keyboard events. All events which are not handled trigger
 * a help window.
 */
void GLViewerWidget::keyPressEvent(QKeyEvent* e)
{
    QPoint middlepoint;

    switch (e->key())
    {
        // next image

        case Qt::Key_N:
        case Qt::Key_Right:
        case Qt::Key_Down:
        case Qt::Key_PageDown:
        case Qt::Key_Space:
        {
            nextImage();
            break;
        }

        // previous image

        case Qt::Key_P:
        case Qt::Key_Left:
        case Qt::Key_Up:
        case Qt::Key_PageUp:
        {
            prevImage();
            break;
        }

        // rotate image

        case Qt::Key_R:
        {
            d->texture->rotate();
            downloadTexture(d->texture);
            update();
            break;
        }

        // terminate image viewer

        case Qt::Key_Escape:
        {
            // clean up: where does this have to be done?

            close();
            break;
        }

        // full screen

        case Qt::Key_F:
        {
            // according to QT documentation, showFullScreen() has some
            // serious issues on window managers that do not follow modern
            // post-ICCCM specifications

            if (isFullScreen())
            {
                d->texture->reset();
                showNormal();     // krazy:exclude=qmethods
            }
            else
            {
                d->texture->reset();
                showFullScreen(); // krazy:exclude=qmethods
            }

            break;
        }

        // reset size and redraw

        case Qt::Key_Z:
        {
            d->texture->reset(true);
            update();

            break;
        }

        // toggle permanent between "show next image" and "zoom" on mousewheel change

        case Qt::Key_C:
        {
            if (d->wheelAction == GLViewerWidget::Private::zoomImage)
            {
                d->wheelAction = GLViewerWidget::Private::changeImage;
            }
            else
            {
                d->wheelAction = GLViewerWidget::Private::zoomImage;
            }

            break;
        }

        // zoom in

        case Qt::Key_Plus:
        {
            middlepoint = QPoint(width() / 2, height() / 2);

            if (d->texture->setNewSize(d->zoomsize))
            {
                downloadTexture(d->texture); // load full resolution image
            }

            zoom(-1, middlepoint, d->zoomfactor_keyboard);

            break;
        }

        // zoom out

        case Qt::Key_Minus:
        {
            middlepoint = QPoint(width() / 2, height() / 2);

            if (d->texture->setNewSize(d->zoomsize))
            {
                downloadTexture(d->texture); // load full resolution image
            }

            zoom(1, middlepoint, d->zoomfactor_keyboard);

            break;
        }

        // zoom to original size

        case Qt::Key_O:
        {
            d->texture->loadFullSize();

            if (d->texture->setNewSize(QSize(0, 0)))
            {
                downloadTexture(d->texture); // load full resolution image
            }

            d->texture->zoomToOriginal();
            update();

            break;
        }

        // toggle temporarily between "show next image" and "zoom" on mousewheel change

        case Qt::Key_Control:
        {
            if (d->wheelAction == GLViewerWidget::Private::zoomImage)
            {
                // scrollwheel changes to the next image

                d->wheelAction = GLViewerWidget::Private::changeImage;
            }
            else
            {
                // scrollwheel does zoom

                d->wheelAction = GLViewerWidget::Private::zoomImage;
                setCursor(d->zoomCursor);
                d->timerMouseMove.stop();
            }

            break;
        }

        // F1 show help dialog to enlighten the user

        case Qt::Key_F1:
        {
            QPointer<DPluginAboutDlg> help = new DPluginAboutDlg(d->plugin);
            help->exec();
            break;
        }

        default:
        {
            break;
        }
    }
}

void GLViewerWidget::keyReleaseEvent(QKeyEvent* e)
{
    switch (e->key())
    {
        case Qt::Key_Plus:
        case Qt::Key_Minus:
        {
            if (!e->isAutoRepeat())
            {
                unsetCursor();

                if (d->texture->setNewSize(QSize(0, 0)))
                {
                    downloadTexture(d->texture); //load full resolution image
                }

                update();
            }
            else
            {
                e->ignore();
            }

            break;
        }

        case Qt::Key_Control:
        {
            if (d->wheelAction == GLViewerWidget::Private::zoomImage)
            {
                d->wheelAction = GLViewerWidget::Private::changeImage;
            }
            else
            {
                d->wheelAction = GLViewerWidget::Private::zoomImage;
                unsetCursor();
                d->timerMouseMove.start(2000);
            }

            break;
        }

        default:
        {
            e->ignore();
            break;
        }
    }
}

/**
 * download texture to video memory
 */
void GLViewerWidget::downloadTexture(GLViewerTexture* const tex)
{
    glBindTexture(GL_TEXTURE_RECTANGLE_NV, tex->textureId());
    // glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
    // glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);

    // uncomment the following line to enable flat shading of texels -> debugging
    // glTexParameterf(GL_TEXTURE_RECTANGLE_NV, GL_TEXTURE_MAG_FILTER,GL_NEAREST);

    // glTexImage2D(GL_TEXTURE_RECTANGLE_NV, 0, GL_RGBA, tex->width(), tex->height(), 0,
    //              GL_RGBA, GL_UNSIGNED_BYTE, tex->data());
}

/**
 * load d->files[file_index] into a texture object if it is not already cached
 * \param file_index index to QStringList d->files
 */
GLViewerTexture* GLViewerWidget::loadImage(int file_index) const
{
    int imod = file_index % CACHESIZE; //index for cache

    if (d->cache[imod].file_index == file_index)
    {
        // image is already cached

        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "image" << file_index << "is already in cache@" << imod;

        return d->cache[imod].texture;
    }
    else
    {
        // image is net yet loaded

        QString f                 = d->files[file_index];
        qCDebug(DIGIKAM_DPLUGIN_GENERIC_LOG) << "loading image" << f << "(idx=" << file_index << ") to cache@" << imod;
        d->cache[imod].file_index = file_index;

        // when loadImage is called the first time, the frame is not yet fullscreen

        QSize size = d->firstImage ? d->screenSize : QSize(width(), height());

        // handle non-loadable images

        if (!d->cache[imod].texture->load(f, size))
        {
            d->cache[imod].texture->load(d->nullImage.toImage());
        }

        d->cache[imod].texture->setViewport(size.width(), size.height());

        return d->cache[imod].texture;
    }
}

void GLViewerWidget::wheelEvent(QWheelEvent* e)
{
    switch (d->wheelAction)
    {
        // mousewheel triggers zoom

        case GLViewerWidget::Private::zoomImage:
        {
            setCursor(d->zoomCursor);
            zoom(e->angleDelta().y(), e->position().toPoint(), d->zoomfactor_scrollwheel);

            break;
        }

        // mousewheel triggers image change

        case GLViewerWidget::Private::changeImage:
        {
            if      (e->angleDelta().y() < 0)
            {
                nextImage();
            }
            else if (e->angleDelta().y() > 0)
            {
                prevImage();
            }

            break;
        }
    }
}

void GLViewerWidget::mousePressEvent(QMouseEvent* e)
{
    // begin zoom
    // scale down d->texture  for fast zooming
    // d->texture will be set to original size on mouse up

    if (d->texture->setNewSize(d->zoomsize))
    {
        // load downsampled image

        downloadTexture(d->texture);
    }

    d->timerMouseMove.stop(); // user is something up to, therefore keep the cursor

    if (e->button() == Qt::LeftButton)
    {
        setCursor(d->moveCursor);
    }

    if (e->button() == Qt::RightButton)
    {
        setCursor(d->zoomCursor);
    }

    d->startdrag    = e->pos();
    d->previous_pos = e->pos();
}

void GLViewerWidget::mouseMoveEvent(QMouseEvent* e)
{
    if      (e->buttons() == Qt::LeftButton)
    {
        // panning

        setCursor(d->moveCursor);
        QPoint diff  = e->pos()-d->startdrag;
        d->texture->move(diff);
        update();
        d->startdrag = e->pos();
    }
    else if (e->buttons() == Qt::RightButton)
    {
        int mdelta = 0;

        // zooming
        //
        // if mouse pointer reached upper or lower boder, special treatment in order
        // to keep zooming enabled in that special case

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
        if (d->previous_pos.y() == e->position().toPoint().y())
        {
            if (e->position().toPoint().y() == 0)
#else
        if (d->previous_pos.y() == e->y())
        {
            if (e->y() == 0)
#endif
            {
                // mouse pointer is at upper edge, therefore assume zoom in

                mdelta = 1;
            }
            else
            {
                // mouse pointer is as lower edge, therefore assume zoom out

                mdelta = -1;
            }
        }
        else
        {
            // mouse pointer is in the middle of the screen, normal operation

#if (QT_VERSION > QT_VERSION_CHECK(5, 99, 0))
            mdelta = d->previous_pos.y() - e->position().toPoint().y();
#else
            mdelta = d->previous_pos.y() - e->y();
#endif
        }

        setCursor(d->zoomCursor);
        zoom(mdelta, d->startdrag, d->zoomfactor_mousemove);
        d->previous_pos = e->pos();
    }
    else
    {
        // no key is pressed while moving mouse
        // don't do anything if ctrl is pressed

        if (d->timerMouseMove.isActive())
        {
            // ctrl is not pressed, no zooming, therefore restore and hide cursor in 2 sec

            unsetCursor();
            d->timerMouseMove.start(2000);
        }
    }

    return;
}

void GLViewerWidget::mouseReleaseEvent(QMouseEvent*)
{
    d->timerMouseMove.start(2000);
    unsetCursor();

    if (d->texture->setNewSize(QSize(0, 0)))
    {
        // load full resolution image

        downloadTexture(d->texture);
    }

    update();
}

/**
 * a double click resets the view (zoom and move)
 */
void GLViewerWidget::mouseDoubleClickEvent(QMouseEvent*)
{
    d->texture->reset(true);
    update();
}

void GLViewerWidget::prevImage()
{

#ifdef PERFORMANCE_ANALYSIS

    GLViewerTimer timer;

#endif

    if (d->file_idx > 0)
    {
        d->file_idx--;
    }
    else
    {
        return;
    }

#ifdef PERFORMANCE_ANALYSIS

    timer.start();

#endif

    d->texture = loadImage(d->file_idx);
    d->texture->reset(true);

#ifdef PERFORMANCE_ANALYSIS

    timer.at("loadImage");

#endif

    downloadTexture(d->texture);

#ifdef PERFORMANCE_ANALYSIS

    timer.at("downloadTexture");

#endif

    update();

#ifdef PERFORMANCE_ANALYSIS

    timer.at("update");

#endif

    //image preloading

    if (d->file_idx > 0)
    {
        loadImage(d->file_idx - 1);
    }
}

void GLViewerWidget::nextImage()
{
#ifdef PERFORMANCE_ANALYSIS

    GLViewerTimer timer;

#endif

    if (d->file_idx < (unsigned int)(d->files.count() - 1))
    {
        d->file_idx++;
    }
    else
    {
        return;
    }

#ifdef PERFORMANCE_ANALYSIS

    timer.start();

#endif

    d->texture = loadImage(d->file_idx);
    d->texture->reset(true);

#ifdef PERFORMANCE_ANALYSIS

    timer.at("loadImage");

#endif

    downloadTexture(d->texture);

#ifdef PERFORMANCE_ANALYSIS

    timer.at("downloadTexture");

#endif

    update();

#ifdef PERFORMANCE_ANALYSIS

    timer.at("updateGL");

#endif

    //image preloading
    if (d->file_idx < ((unsigned int)d->files.count() - 1))
    {
        loadImage(d->file_idx + 1);

#ifdef PERFORMANCE_ANALYSIS

        timer.at("preloading");

#endif

    }
}

/**
 * \param mdelta delta of mouse movement:
 *                              mdelta>0: zoom in
 *                              mdelta<0: zoom out
 *                              mdelta=0: do nothing
 * \param pos position of mouse
 * \param factor zoom factor:scrollwheel needs a higher factor that right click mouse move. factor=1 -> no zoom
 */
void GLViewerWidget::zoom(int mdelta, const QPoint& pos, float factor)
{
    if (mdelta == 0)
    {
        // do nothing
        return;
    }

    if (mdelta > 0)
    {
        // multiplicator for zooming in
        d->delta = factor;
    }

    if (mdelta < 0)
    {
        // multiplicator for zooming out
        d->delta = 2.0 - factor;
    }

    d->texture->zoom(d->delta, pos);
    update();
}

/**
 * being called if user didn't move the mouse for longer than 2 sec
 */
void GLViewerWidget::slotTimeoutMouseMove()
{
    setCursor(Qt::BlankCursor);
}

/**
 * check if OpenGL engine is ready. This function is called from outside the widget.
 * If OpenGL doesn't work correctly, the widget can be destroyed
 * \return OGLstate::oglNoContext No OpenGl context could be retrieved
 * \return OGLstate::oglNoRectangularTexture GLGL_ARB_texture_rectangle is not supported
 * \return OGLstate::oglOK all is fine
 */
OGLstate GLViewerWidget::getOGLstate() const
{
    // No OpenGL context is found. Are the drivers ok?

    if (!isValid())
    {
        return oglNoContext;
    }

    // GL_ARB_texture_rectangle is not supported

    QString s = QString::fromLatin1(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));

    if (!s.contains(QString::fromLatin1("GL_ARB_texture_rectangle"), Qt::CaseInsensitive))
    {
        return oglNoRectangularTexture;
    }

    // Everything is ok!

    return oglOK;
}

/**
 * QGLWidget::isFullscreen() returns true if the internal state is already true
 * but the actually displayed size is still windowed. isReallyFullscreen() returns the
 * value of the visible size.
 * \return true if (screenwidth == widgedwidth)
 */
bool GLViewerWidget::isReallyFullScreen() const
{
    return (width() == d->screenSize.width());
}

} // namespace DigikamGenericGLViewerPlugin
