/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

#include "OpenGLVideo.h"

// Qt includes

#include <QThreadStorage>
#include <QColor>
#include <QGuiApplication>
#include <QScreen>
#include <QSurface>

// Local includes

#include "SurfaceInterop.h"
#include "VideoShader.h"
#include "ShaderManager.h"
#include "GeometryRenderer.h"
#include "OpenGLHelper.h"
#include "digikam_debug.h"

namespace QtAV
{

// FIXME: why crash if inherits both QObject and DPtrPrivate?

class Q_DECL_HIDDEN OpenGLVideoPrivate : public DPtrPrivate<OpenGLVideo>
{
public:

    OpenGLVideoPrivate()
        : ctx               (nullptr),
          manager           (nullptr),
          material          (new VideoMaterial()),
          material_type     (0),
          norm_viewport     (true),
          has_a             (false),
          update_geo        (true),
          tex_target        (0),
          valiad_tex_width  (1.0),
          mesh_type         (OpenGLVideo::RectMesh),
          geometry          (nullptr),
          gr                (nullptr),
          user_shader       (nullptr)
    {
    }

    ~OpenGLVideoPrivate()
    {
        if (material)
        {
            delete material;
            material = nullptr;
        }

        delete geometry;

#if !defined(Q_COMPILER_LAMBDA)

        delete gr;

#endif

    }

    void resetGL()
    {
        ctx = nullptr;

        if (gr)
            gr->updateGeometry(nullptr);

        if (!manager)
            return;

        manager->setParent(nullptr);
        delete manager;
        manager = nullptr;

        if (material)
        {
            delete material;
            material = nullptr;
        }
    }

    // update geometry(vertex array) set attributes or bind VAO/VBO.

    void updateGeometry(VideoShader* shader, const QRectF& t, const QRectF& r);

public:

    QOpenGLContext*         ctx;
    ShaderManager*          manager;
    VideoMaterial*          material;
    qint64                  material_type;
    bool                    norm_viewport;
    bool                    has_a;
    bool                    update_geo;
    int                     tex_target;
    qreal                   valiad_tex_width;
    QSize                   video_size;
    QRectF                  target;
    QRectF                  roi;         ///< including invalid padding width
    OpenGLVideo::MeshType   mesh_type;
    TexturedGeometry*       geometry;
    GeometryRenderer*       gr;
    QRectF                  rect;
    QMatrix4x4              matrix;
    VideoShader*            user_shader;

private:

    Q_DISABLE_COPY(OpenGLVideoPrivate);
};

void OpenGLVideoPrivate::updateGeometry(VideoShader* shader, const QRectF& t, const QRectF& r)
{
    // also check size change for normalizedROI computation if roi is not normalized

    const bool roi_changed = ((valiad_tex_width != material->validTextureWidth()) ||
                              (roi != r) || (video_size != material->frameSize()));
    const int tc           = shader->textureLocationCount();

    if (roi_changed)
    {
        roi              = r;
        valiad_tex_width = material->validTextureWidth();
        video_size       = material->frameSize();
    }

    if (tex_target != shader->textureTarget())
    {
        tex_target = shader->textureTarget();
        update_geo = true;
    }

    bool update_gr = false;
    static QThreadStorage<bool> new_thread;

    if (!new_thread.hasLocalData())
        new_thread.setLocalData(true);

    update_gr      = new_thread.localData();

    if (!gr || update_gr)
    {
        // TODO: only update VAO, not the whole GeometryRenderer

        update_geo                     = true;
        new_thread.setLocalData(false);
        GeometryRenderer* const geordr = new GeometryRenderer(); // local var is captured by lambda
        gr                             = geordr;

#if defined(Q_COMPILER_LAMBDA)

        QObject::connect(QOpenGLContext::currentContext(), &QOpenGLContext::aboutToBeDestroyed,
                         [geordr]
            {
                qCDebug(DIGIKAM_QTAV_LOG).noquote()
                    << QString::asprintf("destroy GeometryRenderer %p", geordr);

                delete geordr;
            }
        );

#endif

    }

    // (-1, -1, 2, 2) must flip y

    QRectF target_rect = (norm_viewport ? QRectF(-1, 1, 2, -2) : rect);

    if (target.isValid())
    {
        if (roi_changed || (target != t))
        {
            target     = t;
            update_geo = true;
/*
            target_rect = target (if valid). // relate to gvf bug?
*/
        }
    }
    else
    {
        if (roi_changed)
        {
            update_geo = true;
        }
    }

    if (!update_geo)
        return;

    delete geometry;
    geometry = nullptr;

    if (mesh_type == OpenGLVideo::SphereMesh)
        geometry = new Sphere();
    else
        geometry = new TexturedGeometry();
/*
    qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("updating geometry...");
*/
    // setTextureCount may change the vertex data. Call it before setRect()

    qCDebug(DIGIKAM_QTAV_LOG) << "target rect: " << target_rect ;

    geometry->setTextureCount((shader->textureTarget() == GL_TEXTURE_RECTANGLE) ? tc : 1);
    geometry->setGeometryRect(target_rect);
    geometry->setTextureRect(material->mapToTexture(0, roi));

    if (shader->textureTarget() == GL_TEXTURE_RECTANGLE)
    {
        for (int i = 1 ; i < tc ; ++i)
        {
            // tc can > planes, but that will compute chroma plane

            geometry->setTextureRect(material->mapToTexture(i, roi), i);
        }
    }

    geometry->create();
    update_geo = false;
    gr->updateGeometry(geometry);
}

OpenGLVideo::OpenGLVideo()
{
    // TODO: system resolution change

    connect(QGuiApplication::instance(), SIGNAL(primaryScreenChanged(QScreen*)),
            this, SLOT(updateViewport()));
}

bool OpenGLVideo::isSupported(VideoFormat::PixelFormat pixfmt)
{
    return ((pixfmt != VideoFormat::Format_RGB48BE) && (pixfmt != VideoFormat::Format_Invalid));
}

void OpenGLVideo::setOpenGLContext(QOpenGLContext* ctx)
{
    DPTR_D(OpenGLVideo);

    if (d.ctx == ctx)
        return;

    qreal b = 0.0;
    qreal c = 0.0;
    qreal h = 0.0;
    qreal s = 0.0;

    if (d.material)
    {
        b          = d.material->brightness();
        c          = d.material->contrast();
        h          = d.material->hue();
        s          = d.material->saturation();
        delete d.material;
        d.material = nullptr;
    }

    d.resetGL(); // TODO: is it ok to destroygl resources in another context?
    d.ctx = ctx;

    if (!ctx)
    {
        return;
    }

    d.material = new VideoMaterial();
    d.material->setBrightness(b);
    d.material->setContrast(c);
    d.material->setHue(h);
    d.material->setSaturation(s);
    d.manager  = ctx->findChild<ShaderManager*>(QLatin1String("__qtav_shader_manager"));

    updateViewport();

    if (d.manager)
        return;

    // TODO: what if ctx is delete?

    d.manager = new ShaderManager(ctx);

    // NOTE: direct connection to make sure there is a valid context.
    // makeCurrent in window.aboutToBeDestroyed()?

    QObject::connect(ctx, SIGNAL(aboutToBeDestroyed()),
                     this, SLOT(resetGL()),
                     Qt::DirectConnection);

    d.manager->setObjectName(QLatin1String("__qtav_shader_manager"));

    /// get gl info here because context is current(qt ensure it)
/*
    const QByteArray extensions(reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
*/
    bool hasGLSL      = QOpenGLShaderProgram::hasOpenGLShaderPrograms();

    qCDebug(DIGIKAM_QTAV_LOG).noquote()
        << QString::asprintf("OpenGL version: %d.%d  hasGLSL: %d",
            ctx->format().majorVersion(), ctx->format().minorVersion(), hasGLSL);

    static bool sInfo = true;

    if (sInfo)
    {
        sInfo = false;

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("GL_VERSION: %s",
                DYGL(glGetString(GL_VERSION)));

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("GL_VENDOR: %s",
                DYGL(glGetString(GL_VENDOR)));

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("GL_RENDERER: %s",
                DYGL(glGetString(GL_RENDERER)));

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("GL_SHADING_LANGUAGE_VERSION: %s",
                DYGL(glGetString(GL_SHADING_LANGUAGE_VERSION)));

        // Check here with current context can ensure the right result.
        // If the first check is in VideoShader/VideoMaterial/decoder or somewhere else,
        // the context can be null

        bool v = OpenGLHelper::isOpenGLES();

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("Is OpenGLES: %d", v);

        v      = OpenGLHelper::isEGL();

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("Is EGL: %d", v);

        const int glsl_ver = OpenGLHelper::GLSLVersion();

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("GLSL version: %d", glsl_ver);

        v      = OpenGLHelper::isPBOSupported();

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("Has PBO: %d", v);

        v      = OpenGLHelper::has16BitTexture();

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("Has 16bit texture: %d", v);

        v      = OpenGLHelper::hasRG();

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("Has RG texture: %d", v);

        qCDebug(DIGIKAM_QTAV_LOG) << ctx->format();
    }
}

QOpenGLContext* OpenGLVideo::openGLContext()
{
    return d_func().ctx;
}

void OpenGLVideo::setCurrentFrame(const VideoFrame& frame)
{
    d_func().material->setCurrentFrame(frame);
    d_func().has_a = frame.format().hasAlpha();
}

void OpenGLVideo::setProjectionMatrixToRect(const QRectF& v)
{
    setViewport(v);
}

void OpenGLVideo::setViewport(const QRectF& r)
{
    DPTR_D(OpenGLVideo);
    d.rect = r;

    if (d.norm_viewport)
    {
        d.matrix.setToIdentity();

        if (d.mesh_type == SphereMesh)
            d.matrix.perspective(45, 1, 0.1, 100); // for sphere
    }
    else
    {
        d.matrix.setToIdentity();
        d.matrix.ortho(r);
        d.update_geo = true; // even true for target_rect != d.rect
    }

    // Mirrored relative to the usual Qt coordinate system with origin in the top left corner.
/*
    mirrored = mat(0, 0) * mat(1, 1) - mat(0, 1) * mat(1, 0) > 0;
*/
    if (d.ctx && (d.ctx == QOpenGLContext::currentContext()))
    {
        DYGL(glViewport(d.rect.x(), d.rect.y(), d.rect.width(), d.rect.height()));
    }
}

void OpenGLVideo::setBrightness(qreal value)
{
    d_func().material->setBrightness(value);
}

void OpenGLVideo::setContrast(qreal value)
{
    d_func().material->setContrast(value);
}

void OpenGLVideo::setHue(qreal value)
{
    d_func().material->setHue(value);
}

void OpenGLVideo::setSaturation(qreal value)
{
    d_func().material->setSaturation(value);
}

void OpenGLVideo::setUserShader(VideoShader* shader)
{
    d_func().user_shader = shader;
}

VideoShader* OpenGLVideo::userShader() const
{
    return d_func().user_shader;
}

void OpenGLVideo::setMeshType(MeshType value)
{
    DPTR_D(OpenGLVideo);

    if (d.mesh_type == value)
        return;

    d.mesh_type  = value;
    d.update_geo = true;

    if ((d.mesh_type == SphereMesh) && d.norm_viewport)
    {
        d.matrix.setToIdentity();
        d.matrix.perspective(45, 1, 0.1, 100); // for sphere
    }
}

OpenGLVideo::MeshType OpenGLVideo::meshType() const
{
    return d_func().mesh_type;
}

void OpenGLVideo::fill(const QColor& color)
{
    DYGL(glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF()));
    DYGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void OpenGLVideo::render(const QRectF &target, const QRectF& roi, const QMatrix4x4& transform)
{
    DPTR_D(OpenGLVideo);

    Q_ASSERT(d.manager);

    Q_EMIT beforeRendering();

    const qint64 mt = d.material->type();

    if (d.material_type != mt)
    {
        qCDebug(DIGIKAM_QTAV_LOG) << "material changed: "
                                  << VideoMaterial::typeName(d.material_type)
                                  << " => " << VideoMaterial::typeName(mt);
        d.material_type = mt;
    }

    // bind first because texture parameters(target) mapped from native buffer is unknown before it

    if (!d.material->bind()) 
        return;

    VideoShader* shader = d.user_shader;

    if (!shader)
    {
        // TODO: print shader type name if changed. prepareMaterial(,sample_code, pp_code)

        shader = d.manager->prepareMaterial(d.material, mt);
    }

    // viewport was used in gpu filters is wrong, qt quick fbo item's is right
    // (so must ensure setProjectionMatrixToRect was called correctly)

    DYGL(glViewport(d.rect.x(), d.rect.y(), d.rect.width(), d.rect.height()));
    shader->update(d.material);
    shader->program()->setUniformValue(shader->matrixLocation(), transform * d.matrix);

    // uniform end. attribute begin

    d.updateGeometry(shader, target, roi);

    // normalize?

    const bool blending = d.has_a;

    if (blending)
    {
        DYGL(glEnable(GL_BLEND));
        gl().BlendFuncSeparate(GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA);
    }
/*
    if (d.mesh_type == OpenGLVideo::SphereMesh)
        DYGL(glEnable(GL_CULL_FACE)); // required for sphere! FIXME: broken in qml and qgvf
*/
    d.gr->render();

    if (blending)
        DYGL(glDisable(GL_BLEND));
/*
    d.shader->program()->release(); // glUseProgram(0)
*/
    d.material->unbind();

    Q_EMIT afterRendering();
}

void OpenGLVideo::resetGL()
{
    qCDebug(DIGIKAM_QTAV_LOG).noquote()
        << QString::asprintf("resetGL %p. from sender %p",
            d_func().manager, sender());

    d_func().resetGL();
}

void OpenGLVideo::updateViewport()
{
    DPTR_D(OpenGLVideo);

    if (!d.ctx)
        return;

    QSizeF surfaceSize = d.ctx->surface()->size();

    // When changing monitors (plugging and unplugging), the screen might sometimes be nullptr!

    if (d.ctx->screen())
    {
        surfaceSize *= d.ctx->screen()->devicePixelRatio();
    }

    surfaceSize *= qApp->devicePixelRatio(); // TODO: window()->devicePixelRatio() is the window screen's

    setProjectionMatrixToRect(QRectF(QPointF(), surfaceSize));
}

} // namespace QtAV
