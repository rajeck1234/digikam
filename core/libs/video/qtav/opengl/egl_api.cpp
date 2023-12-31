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

#define CAPI_LINKAGE EGLAPIENTRY // for functions defined in namespace egl::capi
#define EGL_CAPI_BUILD

/*
#define CAPI_IS_LAZY_RESOLVE 0
*/

// Local includes

#ifndef CAPI_LINK_EGL
#   include "QtAV_capi.h"
#endif

#include "digikam_debug.h"

// include last to avoid covering types later

#include "egl_api.h"  // krazy:exclude=includes

// TODO: winrt resolver use GetModuleHandle first?

namespace egl
{

#ifdef CAPI_LINK_EGL

api::api()               { dll = nullptr; }
api::~api()              {                }
bool api::loaded() const { return true;   }

#else

static const char* names[] =
{

#   ifdef CAPI_TARGET_OS_WIN
#       if defined(QT_CORE_LIB) && !defined(QT_NO_DEBUG)

    "libEGLd",
    "libEGL",

#       else

    "libEGL",
    "libEGLd",

#       endif
#   else

    "EGL",

#   endif

    nullptr
};

class Q_DECL_HIDDEN EGLLib : public ::capi::dso
{
public:

    virtual void* resolve(const char* symbol) override
    {

#   ifndef __MINGW32__ // krazy:exclude=cpp

        return (void*)dso::resolve(symbol);

#   endif

        // from qwindowseglcontext.cpp. Play nice with 32-bit mingw: Try func first, then func@0, func@4, ..., func@64.
        // The def file does not provide any aliases in libEGL and libGLESv2 in these builds which results in exporting function names like eglInitialize@12.
        // This cannot be fixed without breaking binary compatibility. So be flexible here instead.

        void* proc = nullptr;

        for (int n = -4 ; (!proc && (n <= 64)) ; n += 4)
        {
            if (n < 0)
            {
                proc = (void*)dso::resolve(symbol);

                continue;
            }

            char name[512];
            CAPI_SNPRINTF(name, sizeof(name), "%s@%d", symbol, n);
            proc = (void*)dso::resolve(name);

            if (proc)
            {
                qCWarning(DIGIKAM_QTAV_LOG).noquote()
                    << QString::asprintf("%s=>%s\n", symbol, name);
            }
        }

        return proc;
    }
};

typedef EGLLib user_dso;

#   if 1

static const int versions[] =
{
    ::capi::NoVersion,

    // the following line will be replaced by the content of config/egl/version if exists

    1,
    ::capi::EndVersion
};

CAPI_BEGIN_DLL_VER(names, versions, user_dso)

#   else

CAPI_BEGIN_DLL(names, user_dso)

#   endif

// CAPI_DEFINE_RESOLVER(argc, return_type, name, argv_no_name)

#   ifdef EGL_VERSION_1_0

CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglChooseConfig,                  CAPI_ARG5(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglCopyBuffers,                   CAPI_ARG3(EGLDisplay, EGLSurface, EGLNativePixmapType))
CAPI_DEFINE_M_ENTRY(EGLContext,                               EGLAPIENTRY, eglCreateContext,                 CAPI_ARG4(EGLDisplay, EGLConfig, EGLContext, const EGLint*))
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglCreatePbufferSurface,          CAPI_ARG3(EGLDisplay, EGLConfig, const EGLint*))
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglCreatePixmapSurface,           CAPI_ARG4(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint*))
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglCreateWindowSurface,           CAPI_ARG4(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglDestroyContext,                CAPI_ARG2(EGLDisplay, EGLContext))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglDestroySurface,                CAPI_ARG2(EGLDisplay, EGLSurface))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglGetConfigAttrib,               CAPI_ARG4(EGLDisplay, EGLConfig, EGLint, EGLint*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglGetConfigs,                    CAPI_ARG4(EGLDisplay, EGLConfig*, EGLint, EGLint*))
CAPI_DEFINE_M_ENTRY(EGLDisplay,                               EGLAPIENTRY, eglGetCurrentDisplay,             CAPI_ARG0())
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglGetCurrentSurface,             CAPI_ARG1(EGLint))
CAPI_DEFINE_M_ENTRY(EGLDisplay,                               EGLAPIENTRY, eglGetDisplay,                    CAPI_ARG1(EGLNativeDisplayType))
CAPI_DEFINE_M_ENTRY(EGLint,                                   EGLAPIENTRY, eglGetError,                      CAPI_ARG0())
CAPI_DEFINE_M_ENTRY(__eglMustCastToProperFunctionPointerType, EGLAPIENTRY, eglGetProcAddress,                CAPI_ARG1(const char*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglInitialize,                    CAPI_ARG3(EGLDisplay, EGLint*, EGLint*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglMakeCurrent,                   CAPI_ARG4(EGLDisplay, EGLSurface, EGLSurface, EGLContext))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglQueryContext,                  CAPI_ARG4(EGLDisplay, EGLContext, EGLint, EGLint*))
CAPI_DEFINE_M_ENTRY(const char*,                              EGLAPIENTRY, eglQueryString,                   CAPI_ARG2(EGLDisplay, EGLint))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglQuerySurface,                  CAPI_ARG4(EGLDisplay, EGLSurface, EGLint, EGLint*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglSwapBuffers,                   CAPI_ARG2(EGLDisplay, EGLSurface))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglTerminate,                     CAPI_ARG1(EGLDisplay))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglWaitGL,                        CAPI_ARG0())
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglWaitNative,                    CAPI_ARG1(EGLint))

#   endif

#   ifdef EGL_VERSION_1_1

CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglBindTexImage,                  CAPI_ARG3(EGLDisplay, EGLSurface, EGLint))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglReleaseTexImage,               CAPI_ARG3(EGLDisplay, EGLSurface, EGLint))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglSurfaceAttrib,                 CAPI_ARG4(EGLDisplay, EGLSurface, EGLint, EGLint))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglSwapInterval,                  CAPI_ARG2(EGLDisplay, EGLint))

#   endif

#   ifdef EGL_VERSION_1_2

CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglBindAPI,                       CAPI_ARG1(EGLenum))
CAPI_DEFINE_M_ENTRY(EGLenum,                                  EGLAPIENTRY, eglQueryAPI,                      CAPI_ARG0())
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglCreatePbufferFromClientBuffer, CAPI_ARG5(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglReleaseThread,                 CAPI_ARG0())
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglWaitClient,                    CAPI_ARG0())

#   endif

#   ifdef EGL_VERSION_1_4

CAPI_DEFINE_M_ENTRY(EGLContext,                               EGLAPIENTRY, eglGetCurrentContext,             CAPI_ARG0())

#   endif

#   ifdef EGL_VERSION_1_5

CAPI_DEFINE_M_ENTRY(EGLSync,                                  EGLAPIENTRY, eglCreateSync,                    CAPI_ARG3(EGLDisplay, EGLenum, const EGLAttrib*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglDestroySync,                   CAPI_ARG2(EGLDisplay, EGLSync))
CAPI_DEFINE_M_ENTRY(EGLint,                                   EGLAPIENTRY, eglClientWaitSync,                CAPI_ARG4(EGLDisplay, EGLSync, EGLint, EGLTime))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglGetSyncAttrib,                 CAPI_ARG4(EGLDisplay, EGLSync, EGLint, EGLAttrib*))
CAPI_DEFINE_M_ENTRY(EGLImage,                                 EGLAPIENTRY, eglCreateImage,                   CAPI_ARG5(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLAttrib*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglDestroyImage,                  CAPI_ARG2(EGLDisplay, EGLImage))
CAPI_DEFINE_M_ENTRY(EGLDisplay,                               EGLAPIENTRY, eglGetPlatformDisplay,            CAPI_ARG3(EGLenum, void*, const EGLAttrib*))
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglCreatePlatformWindowSurface,   CAPI_ARG4(EGLDisplay, EGLConfig, void*, const EGLAttrib*))
CAPI_DEFINE_M_ENTRY(EGLSurface,                               EGLAPIENTRY, eglCreatePlatformPixmapSurface,   CAPI_ARG4(EGLDisplay, EGLConfig, void*, const EGLAttrib*))
CAPI_DEFINE_M_ENTRY(EGLBoolean,                               EGLAPIENTRY, eglWaitSync,                      CAPI_ARG3(EGLDisplay, EGLSync, EGLint))

#   endif

CAPI_END_DLL()
CAPI_DEFINE_DLL         // cppcheck-suppress[noOperatorEq,noCopyConstructor]

// CAPI_DEFINE(argc, return_type, name, argv_no_name)

#   ifdef EGL_VERSION_1_0

CAPI_DEFINE(EGLBoolean,                               eglChooseConfig,                  CAPI_ARG5(EGLDisplay, const EGLint*, EGLConfig*, EGLint, EGLint*))
CAPI_DEFINE(EGLBoolean,                               eglCopyBuffers,                   CAPI_ARG3(EGLDisplay, EGLSurface, EGLNativePixmapType))
CAPI_DEFINE(EGLContext,                               eglCreateContext,                 CAPI_ARG4(EGLDisplay, EGLConfig, EGLContext, const EGLint*))
CAPI_DEFINE(EGLSurface,                               eglCreatePbufferSurface,          CAPI_ARG3(EGLDisplay, EGLConfig, const EGLint*))
CAPI_DEFINE(EGLSurface,                               eglCreatePixmapSurface,           CAPI_ARG4(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint*))
CAPI_DEFINE(EGLSurface,                               eglCreateWindowSurface,           CAPI_ARG4(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint*))
CAPI_DEFINE(EGLBoolean,                               eglDestroyContext,                CAPI_ARG2(EGLDisplay, EGLContext))
CAPI_DEFINE(EGLBoolean,                               eglDestroySurface,                CAPI_ARG2(EGLDisplay, EGLSurface))
CAPI_DEFINE(EGLBoolean,                               eglGetConfigAttrib,               CAPI_ARG4(EGLDisplay, EGLConfig, EGLint, EGLint*))
CAPI_DEFINE(EGLBoolean,                               eglGetConfigs,                    CAPI_ARG4(EGLDisplay, EGLConfig*, EGLint, EGLint*))
CAPI_DEFINE(EGLDisplay,                               eglGetCurrentDisplay,             CAPI_ARG0())
CAPI_DEFINE(EGLSurface,                               eglGetCurrentSurface,             CAPI_ARG1(EGLint))
CAPI_DEFINE(EGLDisplay,                               eglGetDisplay,                    CAPI_ARG1(EGLNativeDisplayType))
CAPI_DEFINE(EGLint,                                   eglGetError,                      CAPI_ARG0())
CAPI_DEFINE(__eglMustCastToProperFunctionPointerType, eglGetProcAddress,                CAPI_ARG1(const char*))
CAPI_DEFINE(EGLBoolean,                               eglInitialize,                    CAPI_ARG3(EGLDisplay, EGLint*, EGLint*))
CAPI_DEFINE(EGLBoolean,                               eglMakeCurrent,                   CAPI_ARG4(EGLDisplay, EGLSurface, EGLSurface, EGLContext))
CAPI_DEFINE(EGLBoolean,                               eglQueryContext,                  CAPI_ARG4(EGLDisplay, EGLContext, EGLint, EGLint*))
CAPI_DEFINE(const char*,                              eglQueryString,                   CAPI_ARG2(EGLDisplay, EGLint))
CAPI_DEFINE(EGLBoolean,                               eglQuerySurface,                  CAPI_ARG4(EGLDisplay, EGLSurface, EGLint, EGLint*))
CAPI_DEFINE(EGLBoolean,                               eglSwapBuffers,                   CAPI_ARG2(EGLDisplay, EGLSurface))
CAPI_DEFINE(EGLBoolean,                               eglTerminate,                     CAPI_ARG1(EGLDisplay))
CAPI_DEFINE(EGLBoolean,                               eglWaitGL,                        CAPI_ARG0())
CAPI_DEFINE(EGLBoolean,                               eglWaitNative,                    CAPI_ARG1(EGLint))

#   endif

#   ifdef EGL_VERSION_1_1

CAPI_DEFINE(EGLBoolean,                               eglBindTexImage,                  CAPI_ARG3(EGLDisplay, EGLSurface, EGLint))
CAPI_DEFINE(EGLBoolean,                               eglReleaseTexImage,               CAPI_ARG3(EGLDisplay, EGLSurface, EGLint))
CAPI_DEFINE(EGLBoolean,                               eglSurfaceAttrib,                 CAPI_ARG4(EGLDisplay, EGLSurface, EGLint, EGLint))
CAPI_DEFINE(EGLBoolean,                               eglSwapInterval,                  CAPI_ARG2(EGLDisplay, EGLint))

#   endif

#   ifdef EGL_VERSION_1_2

CAPI_DEFINE(EGLBoolean,                               eglBindAPI,                       CAPI_ARG1(EGLenum))
CAPI_DEFINE(EGLenum,                                  eglQueryAPI,                      CAPI_ARG0())
CAPI_DEFINE(EGLSurface,                               eglCreatePbufferFromClientBuffer, CAPI_ARG5(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint*))
CAPI_DEFINE(EGLBoolean,                               eglReleaseThread,                 CAPI_ARG0())
CAPI_DEFINE(EGLBoolean,                               eglWaitClient,                    CAPI_ARG0())

#   endif

#   ifdef EGL_VERSION_1_4

CAPI_DEFINE(EGLContext,                               eglGetCurrentContext,             CAPI_ARG0())

#   endif

#   ifdef EGL_VERSION_1_5

CAPI_DEFINE(EGLSync,                                  eglCreateSync,                    CAPI_ARG3(EGLDisplay, EGLenum, const EGLAttrib*))
CAPI_DEFINE(EGLBoolean,                               eglDestroySync,                   CAPI_ARG2(EGLDisplay, EGLSync))
CAPI_DEFINE(EGLint,                                   eglClientWaitSync,                CAPI_ARG4(EGLDisplay, EGLSync, EGLint, EGLTime))
CAPI_DEFINE(EGLBoolean,                               eglGetSyncAttrib,                 CAPI_ARG4(EGLDisplay, EGLSync, EGLint, EGLAttrib*))
CAPI_DEFINE(EGLImage,                                 eglCreateImage,                   CAPI_ARG5(EGLDisplay, EGLContext, EGLenum, EGLClientBuffer, const EGLAttrib*))
CAPI_DEFINE(EGLBoolean,                               eglDestroyImage,                  CAPI_ARG2(EGLDisplay, EGLImage))
CAPI_DEFINE(EGLDisplay,                               eglGetPlatformDisplay,            CAPI_ARG3(EGLenum,    void*, const EGLAttrib*))
CAPI_DEFINE(EGLSurface,                               eglCreatePlatformWindowSurface,   CAPI_ARG4(EGLDisplay, EGLConfig, void*, const EGLAttrib*))
CAPI_DEFINE(EGLSurface,                               eglCreatePlatformPixmapSurface,   CAPI_ARG4(EGLDisplay, EGLConfig, void*, const EGLAttrib*))
CAPI_DEFINE(EGLBoolean,                               eglWaitSync,                      CAPI_ARG3(EGLDisplay, EGLSync, EGLint))

#   endif

#endif

} // namespace egl
