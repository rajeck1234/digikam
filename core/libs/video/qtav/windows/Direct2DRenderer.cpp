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

#include "VideoRenderer_p.h"

// Qt includes

#include <QWidget>
#include <QResizeEvent>
#include <QLibrary>

// Local includes

#include "QtAV_factory.h"
#include "digikam_debug.h"

// Windows includes

//#define CINTERFACE // http://rxlib.ru/faqs/faqc_en/15596.html                                 // krazy:exclude=insecurenet

#include <sal.h>

#ifndef _Out_writes_bytes_opt_
#   define _Out_writes_bytes_opt_(s)
#endif

#include <initguid.h> // IID_ID2D1Factory
#include <d2d1.h>

// steps: http://msdn.microsoft.com/zh-cn/library/dd317121(v=vs.85).aspx                        // krazy:exclude=insecurenet
// performance: http://msdn.microsoft.com/en-us/library/windows/desktop/dd372260(v=vs.85).aspx  // krazy:exclude=insecurenet
// vlc is helpful
// layer(opacity): http://www.cnblogs.com/graphics/archive/2013/04/15/2781969.html              // krazy:exclude=insecurenet

namespace QtAV
{

template<class Interface>
inline void SafeRelease(Interface** ppInterfaceToRelease)
{
    if (*ppInterfaceToRelease != nullptr)
    {
        (*ppInterfaceToRelease)->Release();
        (*ppInterfaceToRelease) = nullptr;
    }
}

class Direct2DRendererPrivate;

class Q_DECL_HIDDEN Direct2DRenderer : public QWidget,
                                       public VideoRenderer
{
    Q_OBJECT
    DPTR_DECLARE_PRIVATE(Direct2DRenderer)

public:

    Direct2DRenderer(QWidget* const parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags(Qt::Widget));

    VideoRendererId id()                              const override;
    bool isSupported(VideoFormat::PixelFormat pixfmt) const override;

    /* WA_PaintOnScreen: To render outside of Qt's paint system, e.g. If you require
     * native painting primitives, you need to reimplement QWidget::paintEngine() to
     * return 0 and set this flag
     */
    QPaintEngine* paintEngine()                       const override;

    QWidget* widget()                                       override
    {
        return this;
    }

    DECLARE_VIDEO_RENDERER_SIGNALS

protected:

    bool receiveFrame(const VideoFrame& frame)              override;
    void drawBackground()                                   override;
    void drawFrame()                                        override;

    /*
     * usually you don't need to reimplement paintEvent, just drawXXX() is ok. unless you want do all
     * things yourself totally
     */
    void paintEvent(QPaintEvent*)                           override;

    void resizeEvent(QResizeEvent*)                         override;

    // stay on top will change parent, hide then show(windows)

    void showEvent(QShowEvent*)                             override;

    DECLARE_VIDEO_RENDERER_EMIT_METHODS
};

typedef Direct2DRenderer VideoRendererDirect2D;

extern VideoRendererId VideoRendererId_Direct2D;

#if 0

FACTORY_REGISTER_ID_AUTO(VideoRenderer, Direct2D, "Direct2D")

#else

bool RegisterVideoRendererDirect2D_Man()
{
    return VideoRenderer::Register<Direct2DRenderer>(VideoRendererId_Direct2D, "Direct2D");
}

#endif

VideoRendererId Direct2DRenderer::id() const
{
    return VideoRendererId_Direct2D;
}

class Q_DECL_HIDDEN Direct2DRendererPrivate : public VideoRendererPrivate
{
public:

    DPTR_DECLARE_PUBLIC(Direct2DRenderer)

    Direct2DRendererPrivate()
      : VideoRendererPrivate(),
        d2d_factory         (nullptr),
        render_target       (nullptr),
        bitmap              (nullptr),
        bitmap_width        (0),
        bitmap_height       (0),
        interpolation       (D2D1_BITMAP_INTERPOLATION_MODE_LINEAR)
    {
        dll.setFileName(QLatin1String("d2d1"));

        if (!dll.load())
        {
            available = false;

            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("Direct2D is disabled. Failed to load 'd2d1.dll': %s",
                    dll.errorString().toUtf8().constData());

            return;
        }

        typedef HRESULT(WINAPI* D2D1CreateFactory_t)(D2D1_FACTORY_TYPE, REFIID,
                                                     const D2D1_FACTORY_OPTIONS*,
                                                     void** ppIFactory);

        D2D1CreateFactory_t D2D1CreateFactory;
        D2D1CreateFactory = (D2D1CreateFactory_t)dll.resolve("D2D1CreateFactory");

        if (!D2D1CreateFactory)
        {
            available = false;

            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("Direct2D is disabled. Failed to resolve symble 'D2D1CreateFactory': %s",
                    dll.errorString().toUtf8().constData());

            return;
        }

        D2D1_FACTORY_OPTIONS factory_opt =
        {
            D2D1_DEBUG_LEVEL_NONE
        };

        /*
         * d2d is accessed by AVThread and GUI thread, so we use D2D1_FACTORY_TYPE_MULTI_THREADED
         * and let d2d to deal with the thread safe problems. otherwise, if we use
         * D2D1_FACTORY_TYPE_SINGLE_THREADED, we must use lock when copying ID2D1Bitmap and calling EndDraw.
         */

        /// http://msdn.microsoft.com/en-us/library/windows/desktop/dd368104%28v=vs.85%29.aspx      // krazy:exclude=insecurenet

        HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED,
                                       (REFIID)IID_ID2D1Factory,
                                       &factory_opt,
                                       (void**)& d2d_factory);
        if (FAILED(hr))
        {
            available = false;

            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("Direct2D is disabled. Create d2d factory failed");

            return;
        }

        FLOAT dpiX, dpiY;
        d2d_factory->GetDesktopDpi(&dpiX, &dpiY);

        // gcc: extended initializer lists only available with -std=c++11 or -std=gnu++11
        // vc: http://msdn.microsoft.com/zh-cn/library/t8xe60cf(v=vs.80).aspx               // krazy:exclude=insecurenet

        /*
        pixel_format =
        {
            DXGI_FORMAT_B8G8R8A8_UNORM,
            D2D1_ALPHA_MODE_IGNORE
        };
        */

        pixel_format.format    = DXGI_FORMAT_B8G8R8A8_UNORM;
        pixel_format.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED; // D2D1_ALPHA_MODE_IGNORE;

        /*
        bitmap_properties =
        {
            pixel_format,
            dpiX,
            dpiY
        };
        */

        bitmap_properties.pixelFormat = pixel_format;
        bitmap_properties.dpiX        = dpiX;
        bitmap_properties.dpiY        = dpiY;
    }

    ~Direct2DRendererPrivate()
    {
        destroyDeviceResource();
        SafeRelease(&d2d_factory);  // vlc does not call this. why? bug?
        dll.unload();
    }

    bool createDeviceResource()
    {
        DPTR_P(Direct2DRenderer);

        update_background = true;
        destroyDeviceResource();

        //  This method creates resources which are bound to a particular
        //  Direct3D device. It's all centralized here, in case the resources
        //  need to be recreated in case of Direct3D device loss (eg. display
        //  change, remoting, removal of video card, etc).
        //

        // TODO : move to prepare(), or private. how to call less times

        D2D1_RENDER_TARGET_PROPERTIES rtp =
        {
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            pixel_format,
            0,
            0,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT
        };

        D2D1_SIZE_U size =
        {
            (UINT32)p.width(),
            (UINT32)p.height()
        };

        // d.renderer_width, d.renderer_height?

        // Create a Direct2D render target.

        D2D1_HWND_RENDER_TARGET_PROPERTIES hwnd_rtp =
        {
            (HWND)p.winId(),
            size,

            // TODO: what do these mean?

            D2D1_PRESENT_OPTIONS_RETAIN_CONTENTS                       // D2D1_PRESENT_OPTIONS_IMMEDIATELY /* this might need fiddling */
        };

        HRESULT hr = d2d_factory->CreateHwndRenderTarget(&rtp,         // D2D1::RenderTargetProperties() //TODO: vlc set properties
                                                         &hwnd_rtp,    // D2D1::HwndRenderTargetProperties(, size)
                                                         &render_target);
        if (FAILED(hr))
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("Direct2D is disabled. CreateHwndRenderTarget() failed: %d",
                    (int)GetLastError());

            render_target = 0;

            return false;
        }

        SafeRelease(&bitmap);
        prepareBitmap(src_width, src_height); // bitmap depends on render target

        return (hr == S_OK);
    }

    void destroyDeviceResource()
    {
        SafeRelease(&render_target);
        SafeRelease(&bitmap);
    }

    void recreateDeviceResource()
    {
        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("D2DERR_RECREATE_TARGET");

        QMutexLocker locker(&img_mutex);
        Q_UNUSED(locker);

        update_background = true;
        destroyDeviceResource();
        createDeviceResource();
    }

    // create an empty bitmap with given size. if size is equal as current and bitmap already exists, do nothing

    bool prepareBitmap(int w, int h)
    {
        if ((w == bitmap_width) && (h == bitmap_height) && bitmap)
            return true;

        if (!render_target)
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("No render target, bitmap will not be created!!!");

            return false;
        }

        bitmap_width  = w;
        bitmap_height = h;

        qCDebug(DIGIKAM_QTAV_LOG).noquote()
            << QString::asprintf("Resize bitmap to %d x %d", w, h);

        SafeRelease(&bitmap);

        if ((w == 0) || (h == 0))
            return true;

        D2D1_SIZE_U s = { (UINT32)w, (UINT32)h };
        HRESULT hr    = render_target->CreateBitmap(s,
                                                    nullptr,
                                                    0,
                                                    &bitmap_properties,
                                                    &bitmap);
        if (FAILED(hr))
        {
            qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
                << QString::asprintf("Failed to create ID2D1Bitmap (%ld)", hr);

            SafeRelease(&bitmap);
            SafeRelease(&render_target);

            return false;
        }

        return true;
    }

    // it seems that only D2D1_BITMAP_INTERPOLATION_MODE(used in DrawBitmap) matters when drawing an image

    void setupQuality() override
    {
        switch (quality)
        {
            case QtAV::QualityFastest:
            {
                interpolation = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
                render_target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_ALIASED);

                break;
            }

            case QtAV::QualityBest:
            {
                interpolation = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
                render_target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);

                break;
            }

            default:
            {
                interpolation = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
                render_target->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
                render_target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_DEFAULT);

                break;
            }
        }
    }

public:

    ID2D1Factory*                  d2d_factory;
    ID2D1HwndRenderTarget*         render_target;
    D2D1_PIXEL_FORMAT              pixel_format;
    D2D1_BITMAP_PROPERTIES         bitmap_properties;
    ID2D1Bitmap*                   bitmap;
    int                            bitmap_width;
    int                            bitmap_height; // can not use src_width, src height because bitmap not update when they changes
    D2D1_BITMAP_INTERPOLATION_MODE interpolation;
    QLibrary                       dll;
};

Direct2DRenderer::Direct2DRenderer(QWidget* const parent, Qt::WindowFlags f)
    : QWidget      (parent, f),
      VideoRenderer(*new Direct2DRendererPrivate())
{
    DPTR_INIT_PRIVATE(Direct2DRenderer);

    setAcceptDrops(true);
    setFocusPolicy(Qt::StrongFocus);

    /* To rapidly update custom widgets that constantly paint over their entire areas with
     * opaque content, e.g., video streaming widgets, it is better to set the widget's
     * Qt::WA_OpaquePaintEvent, avoiding any unnecessary overhead associated with repainting the
     * widget's background
     */

    setAttribute(Qt::WA_OpaquePaintEvent);
/*
    setAttribute(Qt::WA_NoSystemBackground);
*/
    setAutoFillBackground(false);
    setAttribute(Qt::WA_PaintOnScreen, true);
}

bool Direct2DRenderer::isSupported(VideoFormat::PixelFormat pixfmt) const
{
    return VideoFormat::isRGB(pixfmt);
}

bool Direct2DRenderer::receiveFrame(const VideoFrame& frame)
{
    DPTR_D(Direct2DRenderer);

    if (!frame.isValid())
    {
/*
        d.prepareBitmap(0, 0);
*/
        return false;
    }

    if (!d.prepareBitmap(frame.width(), frame.height()))
        return false;

    HRESULT hr = S_OK;

    // if d2d factory is D2D1_FACTORY_TYPE_SINGLE_THREADED, we need to lock
    // already locked

    if (frame.constBits(0))
        d.video_frame = frame;
    else
        d.video_frame = frame.to(frame.pixelFormat());

    // TODO: if CopyFromMemory() is deep copy, mutex can be avoided
    // if lock is required, do not use locker in if() scope, it will unlock outside the scope
    // TODO: d2d often crash, should we always lock? How about other renderer?

    hr = d.bitmap->CopyFromMemory(nullptr,             // &D2D1::RectU(0, 0, image.width(), image.height()) /*&dstRect, nullptr?*/,
                                  frame.constBits(0),  // data.constData() //msdn: const void*
                                  frame.bytesPerLine(0));

    if (hr != S_OK)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("Failed to copy from memory to bitmap (%ld)", hr);
    }

    updateUi();

    return true;
}

QPaintEngine* Direct2DRenderer::paintEngine() const
{
    return nullptr; // use native engine
}

void Direct2DRenderer::drawBackground()
{
    const QRegion bgRegion(backgroundRegion());

    if (bgRegion.isEmpty())
        return;

    DPTR_D(Direct2DRenderer);

    const QColor bc(backgroundColor());

    D2D1_COLOR_F c =
    {
        (float)bc.red(),
        (float)bc.green(),
        (float)bc.blue(),
        (float)bc.alpha()
    };

    d.render_target->Clear(&c);     // const D2D1_COlOR_F&?

    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd535473(v=vs.85).aspx       // krazy:exclude=insecurenet
/*
    ID2D1SolidColorBrush *brush;
    d.render_target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &brush);
    d.render_target->FillRectangle(D2D1::RectF(0, 0, width(), height()), brush);
*/
}

void Direct2DRenderer::drawFrame()
{
    DPTR_D(Direct2DRenderer);

    if (!d.bitmap || d.out_rect.isEmpty())
        return;

    D2D1_RECT_F out_rect =
    {
        (FLOAT)d.out_rect.x(),
        (FLOAT)d.out_rect.y(),
        (FLOAT)(d.out_rect.x() + d.out_rect.width()), // QRect.right() is x+width-1 for historical reason
        (FLOAT)(d.out_rect.y() + d.out_rect.height())
    };

    QRect roi            = realROI();

    D2D1_RECT_F roi_d2d  =
    {
        (FLOAT)roi.x(),
        (FLOAT)roi.y(),
        (FLOAT)(roi.x() + roi.width()),
        (FLOAT)(roi.y() + roi.height())
    };
/*
    d.render_target->SetTransform
*/
    d.render_target->DrawBitmap(d.bitmap,
                                &out_rect,
                                1,                 // opacity
                                d.interpolation,
                                &roi_d2d);
}

void Direct2DRenderer::paintEvent(QPaintEvent*)
{
    DPTR_D(Direct2DRenderer);

    if (!d.render_target)
    {
        qCWarning(DIGIKAM_QTAV_LOG_WARN).noquote()
            << QString::asprintf("No render target!");

        return;
    }

    // http://www.daimakuai.net/?page_id=1574       // krazy:exclude=insecurenet

    d.render_target->BeginDraw();
    handlePaintEvent();

    HRESULT hr = S_OK;
    {
        // if d2d factory is D2D1_FACTORY_TYPE_SINGLE_THREADED, we need to lock
/*
        QMutexLocker locker(&d.img_mutex);
        Q_UNUSED(locker);
*/
        hr = d.render_target->EndDraw(nullptr, nullptr); //TODO: why it need lock? otherwise crash
    }

    if (hr == D2DERR_RECREATE_TARGET)
    {
        d.recreateDeviceResource();
    }
}

void Direct2DRenderer::resizeEvent(QResizeEvent* e)
{
    resizeRenderer(e->size());
    DPTR_D(Direct2DRenderer);

    d.update_background = true;

    if (d.render_target)
    {
        D2D1_SIZE_U size =
        {
            (UINT32)e->size().width(),
            (UINT32)e->size().height()
        };

        // Note: This method can fail, but it's okay to ignore the
        // error here -- it will be repeated on the next call to
        // EndDraw.

        d.render_target->Resize(&size); // D2D1_SIZE_U&?
    }

    update();
}

void Direct2DRenderer::showEvent(QShowEvent*)
{
    DPTR_D(Direct2DRenderer);

    d.recreateDeviceResource();
}

IMPLEMENT_VIDEO_RENDERER_EMIT_METHODS(Direct2DRenderer)

} // namespace QtAV

#include "Direct2DRenderer.moc"
