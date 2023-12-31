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

#ifndef QTAV_EGL_API_H
#define QTAV_EGL_API_H

// winrt: must define CAPI_LINK_EGL

// no need to include the C header if only functions declared there

#ifndef CAPI_LINK_EGL
#   define EGLAPI // avoid warning and link error
#else

extern "C"
{

// the following line will be replaced by the content of config/EGL/include if exists

#   include <EGL/egl.h>

} // extern "C"

#endif

namespace egl
{

#ifndef CAPI_LINK_EGL
using namespace capi;               // original header is in namespace capi, types are changed
#endif

// For link or NS style. Or load test for class style. api.loaded for class style.

namespace capi
{

    bool loaded();

} // namespace capi

class api_dll;

class api
{
public:

    api();
    virtual ~api();

    /**
     * user may inherits multiple api classes: final::loaded() { return base1::loaded() && base2::loaded();}
     */
    virtual bool loaded() const;

#if !defined(CAPI_LINK_EGL) && !defined(EGL_CAPI_NS)
#   ifdef EGL_VERSION_1_0

    EGLBoolean  eglChooseConfig(EGLDisplay dpy, const EGLint* attrib_list, EGLConfig* configs, EGLint config_size, EGLint* num_config);
    EGLBoolean  eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
    EGLContext  eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint* attrib_list);
    EGLSurface  eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint* attrib_list);
    EGLSurface  eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint* attrib_list);
    EGLSurface  eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint* attrib_list);
    EGLBoolean  eglDestroyContext(EGLDisplay dpy, EGLContext ctx);
    EGLBoolean  eglDestroySurface(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean  eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint* value);
    EGLBoolean  eglGetConfigs(EGLDisplay dpy, EGLConfig* configs, EGLint config_size, EGLint* num_config);
    EGLDisplay  eglGetCurrentDisplay();
    EGLSurface  eglGetCurrentSurface(EGLint readdraw);
    EGLDisplay  eglGetDisplay(EGLNativeDisplayType display_id);
    EGLint      eglGetError();
    __eglMustCastToProperFunctionPointerType eglGetProcAddress(const char* procname);
    EGLBoolean  eglInitialize(EGLDisplay dpy, EGLint* major, EGLint* minor);
    EGLBoolean  eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
    EGLBoolean  eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint* value);
    const char* eglQueryString(EGLDisplay dpy, EGLint name);
    EGLBoolean  eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint* value);
    EGLBoolean  eglSwapBuffers(EGLDisplay dpy, EGLSurface surface);
    EGLBoolean  eglTerminate(EGLDisplay dpy);
    EGLBoolean  eglWaitGL();
    EGLBoolean  eglWaitNative(EGLint engine);

#endif

#ifdef EGL_VERSION_1_1

    EGLBoolean  eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
    EGLBoolean  eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
    EGLBoolean  eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
    EGLBoolean  eglSwapInterval(EGLDisplay dpy, EGLint interval);

#endif

#ifdef EGL_VERSION_1_2

    EGLBoolean  eglBindAPI(EGLenum api);
    EGLenum     eglQueryAPI();
    EGLSurface  eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint * attrib_list);
    EGLBoolean  eglReleaseThread();
    EGLBoolean  eglWaitClient();

#endif

#ifdef EGL_VERSION_1_4

    EGLContext  eglGetCurrentContext();

#endif

#ifdef EGL_VERSION_1_5

    EGLSync     eglCreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib* attrib_list);
    EGLBoolean  eglDestroySync(EGLDisplay dpy, EGLSync sync);
    EGLint      eglClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout);
    EGLBoolean  eglGetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib* value);
    EGLImage    eglCreateImage(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLAttrib* attrib_list);
    EGLBoolean  eglDestroyImage(EGLDisplay dpy, EGLImage image);
    EGLDisplay  eglGetPlatformDisplay(EGLenum platform, void* native_display, const EGLAttrib* attrib_list);
    EGLSurface  eglCreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void* native_window, const EGLAttrib* attrib_list);
    EGLSurface  eglCreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void* native_pixmap, const EGLAttrib* attrib_list);
    EGLBoolean  eglWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags);

#endif

#endif

private:

    api_dll* dll = nullptr;

private:

    api(const api&)            = delete;
    api& operator=(const api&) = delete;
};

} // namespace egl

#ifndef EGL_CAPI_BUILD // avoid ambiguous in egl_api.cpp
#   if defined(EGL_CAPI_NS) && !defined(CAPI_LINK_EGL)

using namespace egl::capi;

#   else

using namespace egl;

#   endif

#endif

#endif // QTAV_EGL_API_H
