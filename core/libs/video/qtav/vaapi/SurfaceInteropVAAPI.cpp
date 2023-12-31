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

/// The EGL+DMA part is from mpv and under LGPL

#include "SurfaceInteropVAAPI.h"

#ifndef QT_NO_OPENGL
#   include "OpenGLHelper.h"
#   include "VideoFrame.h"
#   include "digikam_debug.h"

#   if VA_X11_INTEROP
#       ifdef QT_X11EXTRAS_LIB
#           include <QtX11Extras/QX11Info>
#       endif
#       include <va/va_x11.h>
#       if QTAV_HAVE(EGL_CAPI)
#           define EGL_CAPI_NS
#           include "egl_api.h"
#           include <EGL/eglext.h>
#           if VA_CHECK_VERSION(0, 33, 0)
#               include <va/va_drmcommon.h>
#           else

/** \brief Kernel DRM buffer memory type.  */

#               define VA_SURFACE_ATTRIB_MEM_TYPE_KERNEL_DRM       0x10000000

/** \brief DRM PRIME memory type. */

#               define VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME        0x20000000
#           endif
#       endif
#       if !defined(QT_OPENGL_ES_2)
#           include <GL/glx.h>
#       endif
#   endif

namespace QtAV
{

VideoFormat::PixelFormat pixelFormatFromVA(uint32_t fourcc);

namespace vaapi
{

// 2013 https://www.khronos.org/registry/egl/extensions/EXT/EGL_EXT_image_dma_buf_import.txt

bool checkEGL_DMA()
{

#   ifndef QT_NO_OPENGL

    static const char* eglexts[] =
    {
        "EGL_EXT_image_dma_buf_import",
        nullptr
    };

    return OpenGLHelper::hasExtensionEGL(eglexts);

#   endif

    return false;
}

// https://www.khronos.org/registry/egl/extensions/KHR/EGL_KHR_image_pixmap.txt

bool checkEGL_Pixmap()
{

#   ifndef QT_NO_OPENGL

    static const char* eglexts[] =
    {
        "EGL_KHR_image_pixmap",
        nullptr
    };

    return OpenGLHelper::hasExtensionEGL(eglexts);

#   endif

    return false;
}

void SurfaceInteropVAAPI::setSurface(const surface_ptr& surface, int w, int h)
{
    m_surface    = surface;
    frame_width  = (w ? w : surface->width());
    frame_height = (h ? h : surface->height());
}

void* SurfaceInteropVAAPI::map(SurfaceType type, const VideoFormat& fmt, void* handle, int plane)
{
    if (!handle)
        return nullptr;
/*
    if (!fmt.isRGB())
        return 0;
*/
    if (!m_surface)
        return nullptr;

    if      (type == GLTextureSurface)
    {
        if (m_resource->map(m_surface, *((GLuint*)handle), frame_width, frame_height, plane))
            return handle;

        return nullptr;
    }
    else if (type == HostMemorySurface)
    {
        return mapToHost(fmt, handle, plane);
    }

    return nullptr;
}

void SurfaceInteropVAAPI::unmap(void* handle)
{
    m_resource->unmap(m_surface, *((GLuint*)handle));
}

void* SurfaceInteropVAAPI::mapToHost(const VideoFormat& format, void* handle, int plane)
{
    Q_UNUSED(plane);
    VAImage image;

    static const unsigned int fcc[] =
    {
        VA_FOURCC_NV12,
        VA_FOURCC_YV12,
        VA_FOURCC_IYUV,
        0
    };

    va_new_image(m_surface->vadisplay(), fcc, &image, m_surface->width(), m_surface->height());

    if (image.image_id == VA_INVALID_ID)
        return nullptr;

    void* p_base = nullptr;
    VA_ENSURE(vaGetImage(m_surface->vadisplay(), m_surface->get(), 0, 0,
              m_surface->width(), m_surface->height(), image.image_id), nullptr);

    // TODO: destroy image before return

    VA_ENSURE(vaMapBuffer(m_surface->vadisplay(), image.buf, &p_base), nullptr);

    VideoFormat::PixelFormat pixfmt = pixelFormatFromVA(image.format.fourcc);
    bool swap_uv                    = (image.format.fourcc != VA_FOURCC_NV12);

    if (pixfmt == VideoFormat::Format_Invalid)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("unsupported vaapi pixel format: %#x",
                image.format.fourcc);

        VA_ENSURE(vaDestroyImage(m_surface->vadisplay(), image.image_id), nullptr);

        return nullptr;
    }

    const VideoFormat fmt(pixfmt);
    uint8_t* src[3] = { nullptr };
    int pitch[3]    = { 0 };

    for (int i = 0 ; i < fmt.planeCount() ; ++i)
    {
        src[i]   = (uint8_t*)p_base + image.offsets[i];
        pitch[i] = image.pitches[i];
    }

    VideoFrame frame = VideoFrame::fromGPU(fmt, frame_width, frame_height,
                                           m_surface->height(), src, pitch, true, swap_uv);

    if (format != fmt)
        frame = frame.to(format);

    VAWARN(vaUnmapBuffer(m_surface->vadisplay(), image.buf));
    VAWARN(vaDestroyImage(m_surface->vadisplay(), image.image_id));
    image.image_id       = VA_INVALID_ID;
    VideoFrame* const f  = reinterpret_cast<VideoFrame*>(handle);
    frame.setTimestamp(f->timestamp());
    *f                   = frame;

    return f;
}

#   ifndef QT_NO_OPENGL

surface_glx_ptr GLXInteropResource::surfaceGLX(const display_ptr& dpy, GLuint tex)
{
    surface_glx_ptr glx = glx_surfaces[tex];

    if (glx)
        return glx;

    glx = surface_glx_ptr(new surface_glx_t(dpy));

    if (!glx->create(tex))
        return surface_glx_ptr();

    glx_surfaces[tex] = glx;

    return glx;
}

bool GLXInteropResource::map(const surface_ptr& surface, GLuint tex, int w, int h, int)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
    surface_glx_ptr glx = surfaceGLX(surface->display(), tex);

    if (!glx)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("Fail to create vaapi glx surface");

        return false;
    }

    if (!glx->copy(surface))
        return false;

    VAWARN(vaSyncSurface(surface->vadisplay(), surface->get()));

    return true;
}

#   endif // QT_NO_OPENGL

#   if VA_X11_INTEROP

class Q_DECL_HIDDEN X11
{
protected:

    Display* display = nullptr;

public:

    X11()
        : display(nullptr),
          pixmap(0)
    {
    }

    virtual ~X11()
    {
        if (pixmap)
            XFreePixmap((::Display*)display, pixmap);

        pixmap = 0;
    }

    virtual Display* ensureGL()           = 0;
    virtual bool bindPixmap(int w, int h) = 0;
    virtual void bindTexture()            = 0;

public:

    Pixmap pixmap = 0;

protected:

    int createPixmap(int w, int h)
    {
        if (pixmap)
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("XFreePixmap");
            XFreePixmap((::Display*)display, pixmap);
            pixmap = 0;
        }

        XWindowAttributes xwa;
        XGetWindowAttributes((::Display*)display, DefaultRootWindow((::Display*)display), &xwa);
        pixmap = XCreatePixmap((::Display*)display, DefaultRootWindow((::Display*)display), w, h, xwa.depth);

        // mpv always use 24 bpp

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("XCreatePixmap %lu: %dx%d, depth: %d",
                pixmap, w, h, xwa.depth);

        if (!pixmap)
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("X11InteropResource could not create pixmap");

            return 0;
        }

        return xwa.depth;
    }

private:

    Q_DISABLE_COPY(X11);
};

#   define RESOLVE_FUNC(func, resolver, st) do {                        \
    if (!func) {                                                        \
        typedef void (*ft)(void);                                       \
        ft f   = resolver((st)#func);                                   \
        if (!f) {                                                       \
            qCWarning(DIGIKAM_QTAV_LOG) << #func " is not available";   \
            return 0;                                                   \
        }                                                               \
        ft* fp = (ft*)(&func);                                          \
        *fp    = f;                                                     \
    }} while(0)

#   define RESOLVE_EGL(func) RESOLVE_FUNC(func, eglGetProcAddress, const char*)

#   define RESOLVE_GLX(func) RESOLVE_FUNC(func, glXGetProcAddressARB, const GLubyte*)

#   if QTAV_HAVE(EGL_CAPI)

//static PFNEGLQUERYNATIVEDISPLAYNVPROC eglQueryNativeDisplayNV = nullptr;

static PFNEGLCREATEIMAGEKHRPROC  eglCreateImageKHR                      = nullptr;
static PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR                     = nullptr;

#   ifndef GL_OES_EGL_image

typedef void* GLeglImageOES;
typedef void (EGLAPIENTRYP PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)(GLenum target, GLeglImageOES image);

#   endif

static PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;

class Q_DECL_HIDDEN X11_EGL final: public X11
{
public:

    X11_EGL()
        : dpy(EGL_NO_DISPLAY)
    {
        for (unsigned i = 0 ; i < sizeof(image) / sizeof(image[0]) ; ++i)
            image[i] = EGL_NO_IMAGE_KHR;
    }

    ~X11_EGL()
    {
        for (unsigned i = 0 ; i < sizeof(image) / sizeof(image[0]) ; ++i)
        {
            if (image[i] != EGL_NO_IMAGE_KHR)
            {
                EGL_WARN(eglDestroyImageKHR(dpy, image[i]));
            }
        }
    }

    Display* ensureGL() override
    {
        if (display && eglCreateImageKHR && glEGLImageTargetTexture2DOES)
            return display;

        // we must use the native display egl used, otherwise eglCreateImageKHR will fail.

#   ifdef QT_X11EXTRAS_LIB

        display = (Display*)QX11Info::display();

#   endif
/*
        RESOLVE_EGL(eglQueryNativeDisplayNV);
*/
        RESOLVE_EGL(glEGLImageTargetTexture2DOES);
        RESOLVE_EGL(eglCreateImageKHR);
        RESOLVE_EGL(eglDestroyImageKHR);

        return display;
    }

    bool bindPixmap(int w, int h) override
    {
        if (!createPixmap(w, h))
            return false;

        if (dpy == EGL_NO_DISPLAY)
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("eglGetCurrentDisplay");

            dpy = eglGetCurrentDisplay();
        }

        EGL_ENSURE(image[0] = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_PIXMAP_KHR,
                                                (EGLClientBuffer)pixmap, nullptr), false);

        return true;
    }

    void bindTexture() override
    {
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image[0]);
    }

public:

    EGLDisplay  dpy;
    EGLImageKHR image[4];

private:

    Q_DISABLE_COPY(X11_EGL);
};

#   endif // QTAV_HAVE(EGL_CAPI)

#   if !defined(QT_OPENGL_ES_2)

typedef void (*glXBindTexImageEXT_t)(Display* dpy, GLXDrawable draw, int buffer, int* a);
typedef void (*glXReleaseTexImageEXT_t)(Display* dpy, GLXDrawable draw, int buffer);

static glXReleaseTexImageEXT_t glXReleaseTexImageEXT = nullptr;
static glXBindTexImageEXT_t glXBindTexImageEXT       = nullptr;

class Q_DECL_HIDDEN X11_GLX final : public X11
{
public:

    X11_GLX()
        : fbc(nullptr),
          glxpixmap(0)
    {
    }

    ~X11_GLX()
    {
        if (glxpixmap)
        {
            // TODO: does the thread matters?

            glXReleaseTexImageEXT(display, glxpixmap, GLX_FRONT_EXT);
            XSync((::Display*)display, False);
            glXDestroyPixmap((::Display*)display, glxpixmap);
        }

        glxpixmap = 0;
    }

    Display* ensureGL() override
    {
        if (fbc && display)
            return display;

        if (!display)
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("glXGetCurrentDisplay");

            display = (Display*)glXGetCurrentDisplay(); // use for all and not x11info

            if (!display)
                return nullptr;
        }

        int xscr           = DefaultScreen(display);
        const char* glxext = glXQueryExtensionsString((::Display*)display, xscr);

        if (!glxext || !strstr(glxext, "GLX_EXT_texture_from_pixmap"))
            return nullptr;

        RESOLVE_GLX(glXBindTexImageEXT);
        RESOLVE_GLX(glXReleaseTexImageEXT);

        int attribs[] =
        {
            GLX_RENDER_TYPE,                 GLX_RGBA_BIT,  // xbmc
            GLX_X_RENDERABLE,                True,          // xbmc
            GLX_BIND_TO_TEXTURE_RGBA_EXT,    True,
            GLX_DRAWABLE_TYPE,               GLX_PIXMAP_BIT,
            GLX_BIND_TO_TEXTURE_TARGETS_EXT, GLX_TEXTURE_2D_BIT_EXT,
            GLX_Y_INVERTED_EXT,              True,
            GLX_DOUBLEBUFFER,                False,
            GLX_RED_SIZE,                    8,
            GLX_GREEN_SIZE,                  8,
            GLX_BLUE_SIZE,                   8,
            GLX_ALPHA_SIZE,                  8,             // 0 for 24 bpp(vdpau)? mpv is 0
            None
        };

        int fbcount;
        GLXFBConfig* const fbcs = glXChooseFBConfig((::Display*)display, xscr, attribs, &fbcount);

        if (!fbcount)
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("No texture-from-pixmap support");

            return nullptr;
        }

        if (fbcount)            // cppcheck-suppress knownConditionTrueFalse
            fbc = fbcs[0];

        XFree(fbcs);

        return display;
    }

    bool bindPixmap(int w, int h) override
    {
        const int depth = createPixmap(w, h);

        if (depth <= 0)
            return false;

        const int attribs[] =
        {
            GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
            GLX_TEXTURE_FORMAT_EXT, (depth == 32) ? GLX_TEXTURE_FORMAT_RGBA_EXT : GLX_TEXTURE_FORMAT_RGB_EXT,
            GLX_MIPMAP_TEXTURE_EXT, False,
            None,
        };

        glxpixmap = glXCreatePixmap((::Display*)display, fbc, pixmap, attribs);

        return true;
    }

    void bindTexture() override
    {
        glXBindTexImageEXT(display, glxpixmap, GLX_FRONT_EXT, nullptr);
    }

public:

    GLXFBConfig fbc;
    GLXPixmap   glxpixmap;

private:

    Q_DISABLE_COPY(X11_GLX);
};

#   endif // !defined(QT_OPENGL_ES_2)

X11InteropResource::X11InteropResource()
    : InteropResource(),
      VAAPI_X11      (),
      xdisplay       (nullptr),
      width          (0),
      height         (0),
      x11            (nullptr)
{
    qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("X11InteropResource");
}

X11InteropResource::~X11InteropResource()
{
    delete x11;
}

bool X11InteropResource::ensurePixmaps(int w, int h)
{
    if ((width == w) && (height == h))
        return true;

    if (!x11)
    {

#   if QTAV_HAVE(EGL_CAPI)

        if (OpenGLHelper::isEGL())
        {
            x11 = new X11_EGL();
        }
        else

#   endif

        {

#   if !defined(QT_OPENGL_ES_2)

            // TODO: check GLX_EXT_texture_from_pixmap

            x11 = new X11_GLX();

#   endif

        }
    }

    if (!x11)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("no EGL and GLX interop (TFP) support");

        return false;
    }

    xdisplay  = x11->ensureGL();

    if (!xdisplay)
        return false;

    if (!x11->bindPixmap(w, h))
        return false;

    width  = w;
    height = h;

    return true;
}

bool X11InteropResource::map(const surface_ptr& surface, GLuint tex, int w, int h, int)
{
    if ((surface->width() <= 0) || (surface->height() <= 0))
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("invalid surface size");

        return false;
    }

    if (!ensurePixmaps(w, h)) // pixmap with frame size
        return false;

    VAWARN(vaSyncSurface(surface->vadisplay(), surface->get()));

    // FIXME: invalid surface at the first time vaPutSurface is called.
    // If return false, vaPutSurface will always fail, why?

    VAWARN(vaPutSurface(surface->vadisplay(), surface->get(), x11->pixmap,
                        0, 0, w, h,
                        0, 0, w, h,
                        nullptr, 0, VA_FRAME_PICTURE | surface->colorSpace())
    );

    XSync((::Display*)xdisplay, False);
    DYGL(glBindTexture(GL_TEXTURE_2D, tex));
    x11->bindTexture();
    DYGL(glBindTexture(GL_TEXTURE_2D, 0));

    return true;
}

bool X11InteropResource::unmap(const surface_ptr& surface, GLuint tex)
{
    Q_UNUSED(surface);
    Q_UNUSED(tex);

    // can not call glXReleaseTexImageEXT otherwise the
    // texture will containts no image data

    return true;
}

#   endif //VA_X11_INTEROP

#   if QTAV_HAVE(EGL_CAPI)
#       ifndef EGL_LINUX_DMA_BUF_EXT
#           define EGL_LINUX_DMA_BUF_EXT 0x3270
#           define EGL_LINUX_DRM_FOURCC_EXT          0x3271
#           define EGL_DMA_BUF_PLANE0_FD_EXT         0x3272
#           define EGL_DMA_BUF_PLANE0_OFFSET_EXT     0x3273
#           define EGL_DMA_BUF_PLANE0_PITCH_EXT      0x3274
#       endif

// 2010 https://www.khronos.org/registry/egl/extensions/MESA/EGL_MESA_drm_image.txt:
// only support EGL_DRM_BUFFER_FORMAT_ARGB32_MESA

class Q_DECL_HIDDEN EGL
{
public:

    EGL()
        : dpy(EGL_NO_DISPLAY)
    {
        for (unsigned i = 0 ; (i < sizeof(image) / sizeof(image[0])) ; ++i)
            image[i] = EGL_NO_IMAGE_KHR;
    }

    ~EGL()
    {
        for (unsigned i = 0 ; (i < sizeof(image) / sizeof(image[0])) ; ++i)
        {
            destroyImages(i);
        }
    }

    void destroyImages(int plane)
    {
/*
        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("destroyImage %d image:%p,
                this: %p", plane, image,this);
*/
        if (image[plane] != EGL_NO_IMAGE_KHR)
        {
            EGL_WARN(eglDestroyImageKHR(dpy, image[plane]));
            image[plane] = EGL_NO_IMAGE_KHR;
        }
    }

    bool ensureGL()
    {
        if (eglCreateImageKHR && eglDestroyImageKHR && glEGLImageTargetTexture2DOES)
            return true;

        RESOLVE_EGL(glEGLImageTargetTexture2DOES);
        RESOLVE_EGL(eglCreateImageKHR);
        RESOLVE_EGL(eglDestroyImageKHR);

        return (eglCreateImageKHR && eglDestroyImageKHR && glEGLImageTargetTexture2DOES);
    }

    bool bindImage(int plane, const EGLint *attrib_list)
    {
        if (dpy == EGL_NO_DISPLAY)
        {
            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("eglGetCurrentDisplay");

            dpy = eglGetCurrentDisplay();
        }

        EGL_ENSURE(image[plane] = eglCreateImageKHR(eglGetCurrentDisplay(),
                                                    EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT,
                                                    nullptr, attrib_list), false);

        return true;
    }

    void bindTexture(int plane)
    {
        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image[plane]);
    }

public:

    EGLDisplay  dpy;
    EGLImageKHR image[4];
};

EGLInteropResource::EGLInteropResource()
    : InteropResource(),
      vabuf_handle   (0),
      egl            (nullptr)
{
    va_image.buf = va_image.image_id = VA_INVALID_ID;
}

EGLInteropResource::~EGLInteropResource()
{
    delete egl; // TODO: thread
}

bool EGLInteropResource::map(const surface_ptr& surface, GLuint tex, int w, int h, int plane)
{
    if (!ensure())
        return false;

    if (va_image.image_id == VA_INVALID_ID)
    {
        // TODO: try vaGetImage. it's yuv420p. RG texture is not supported by gles2,
        // so let's use yuv420p, or change the shader

        VA_ENSURE(vaDeriveImage(surface->vadisplay(), surface->get(), &va_image), false);
    }

    if (!vabuf_handle)
    {
        va_0_38::VABufferInfo vabuf;
        memset(&vabuf, 0, sizeof(vabuf));
        vabuf.mem_type = VA_SURFACE_ATTRIB_MEM_TYPE_DRM_PRIME;
        VA_ENSURE(va_0_38::vaAcquireBufferHandle(surface->vadisplay(), va_image.buf, &vabuf), false);
        vabuf_handle   = vabuf.handle;
    }

    // (it would be nice if we could use EGL_IMAGE_INTERNAL_FORMAT_EXT)

#   define FOURCC(a,b,c,d) ((a) | ((b) << 8) | ((c) << 16) | ((unsigned)(d) << 24))

    static const int drm_fmts[] =
    {
        FOURCC('R', '8', ' ', ' '),   // DRM_FORMAT_R8
        FOURCC('G', 'R', '8', '8'),   // DRM_FORMAT_GR88. RG88 does not work
        FOURCC('R', 'G', '2', '4'),   // DRM_FORMAT_RGB888
        FOURCC('R', 'A', '2', '4')    // DRM_FORMAT_RGBA8888
    };

#   undef FOURCC

    // TODO: yv12 swap uv

    const VideoFormat fmt(pixelFormatFromVA(va_image.format.fourcc));

    const EGLint attribs[] =
    {
        EGL_LINUX_DRM_FOURCC_EXT,       drm_fmts[fmt.bytesPerPixel(plane) - 1],
        EGL_WIDTH,                      fmt.width(w, plane),
        EGL_HEIGHT,                     fmt.height(h, plane),
        EGL_DMA_BUF_PLANE0_FD_EXT,      (EGLint)vabuf_handle,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT,  (EGLint)va_image.offsets[plane],
        EGL_DMA_BUF_PLANE0_PITCH_EXT,   (EGLint)va_image.pitches[plane],
        EGL_NONE
    };

    if (!egl->bindImage(plane, attribs))
        return false;

    DYGL(glBindTexture(GL_TEXTURE_2D, tex));
    egl->bindTexture(plane);
    DYGL(glBindTexture(GL_TEXTURE_2D, 0));
    mapped.insert(tex, plane);

    return true;
}

bool EGLInteropResource::unmap(const surface_ptr& surface, GLuint tex)
{
    if (!egl)
        return false;

    if (!mapped.contains(tex))
    {
        if (mapped.isEmpty())
        {
            destroy(surface->vadisplay());
        }

        return false;
    }

    const int plane = mapped.value(tex);
    egl->destroyImages(plane);
    mapped.remove(tex);

    if (mapped.isEmpty())
    {
        destroy(surface->vadisplay());
    }

    return true;
}

void EGLInteropResource::destroy(VADisplay va_dpy)
{
    if (!va_dpy)
        return;

    if (va_image.buf != VA_INVALID_ID)
    {
        VAWARN(va_0_38::vaReleaseBufferHandle(va_dpy, va_image.buf));
        va_image.buf = VA_INVALID_ID;
        vabuf_handle = 0;
/*
        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("vabuf_handle: %#x",
                vabuf_handle);
*/
    }

    if (va_image.image_id != VA_INVALID_ID)
    {
        VAWARN(vaDestroyImage(va_dpy, va_image.image_id));
        va_image.image_id = VA_INVALID_ID;
    }
}

bool EGLInteropResource::ensure()
{
    if (egl)
        return true;

#   if QTAV_HAVE(EGL_CAPI)

    if (!OpenGLHelper::isEGL())
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("Not using EGL");

        return false;
    }

#   else

    qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
        << QString::asprintf("build QtAV with capi is required");

    return false;

#endif // QTAV_HAVE(EGL_CAPI)

    static bool has_ext = false;

    if (!has_ext)
    {
        if (!vaapi::checkEGL_DMA())
            return false;

        static const char* glexts[] =
        {
            "GL_OES_EGL_image",
            nullptr
        };

        if (!OpenGLHelper::hasExtension(glexts))
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("missing extension: GL_OES_EGL_image");

            return false;
        }

        has_ext = true;
    }

    if (!egl)
        egl = new EGL();

    if (!egl->ensureGL())
        return false;

    return true;
}

#   endif // QTAV_HAVE(EGL_CAPI)

} // namespace QtAV

} // namespace vaapi

#endif // QT_NO_OPENGL
