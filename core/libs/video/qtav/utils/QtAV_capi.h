/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *               Use C API in C++ dynamically and no link. Header only.
 *               Use it with a code generation tool: https://github.com/wang-bin/mkapi
 *               No class based implementation: https://github.com/wang-bin/dllapi.
 *               Limitation: can not reload library.
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

/**
 *
 * This header only tool helps you use C APIs in a shared library by dynamically loading instead of linking against it with minimal efforts.
 *
 * Only depends on std C++
 *
 * Here is a simple zlib example, if you want use zlib functions, inherits class zlib::api.
 *
 * >define a class with zlib functions in zlib_api.h
 *
 *     #ifndef CAPI_LINK_ZLIB
 *     namespace zlib { //need a unique namespace
 *     namespace capi {
 *     #else
 *     extern "C" {
 *     #endif
 *     #include "zlib.h" //// we need some types define there. otherwise we can remove this
 *     #ifndef CAPI_LINK_ZLIB
 *     }
 *     #endif
 *     }
 *
 *     namespace zlib { //need a unique namespace
 *     #ifndef CAPI_LINK_ZLIB
 *     using namespace capi;
 *     #endif
 *     class api_dll; //must use this name
 *     class api //must use this name
 *     {
 *         api_dll *dll;
 *     public:
 *         api();
 *         virtual ~api();
 *         virtual bool loaded() const;
 *     #if !defined(CAPI_LINK_ZLIB) && !defined(ZLIB_CAPI_NS)
 *         const char* zlibVersion();
 *         const char* zError(int);
 *     #endif
 *     };
 *     } //namespace zlib
 *     #ifdef ZLIB_CAPI_NS
 *     using namespace zlib::capi;
 *     #else
 *     using namespace zlib;
 *     #endif
 *
 * `zlib_api.h` is the header you will use
 *
 * >zlib_api.cpp (some code can be generated from  tools/mkapi)
 *
 *     #define DEBUG //log dll load and symbol resolve
 *     //#define CAPI_IS_LAZY_RESOLVE 0 //define it will resolve all symbols in constructor
 *     #include "QtAV_capi.h"
 *     #include "zlib_api.h" //include last because zlib.h was in namespace capi to avoid covering types later
 *
 *     namespace zlib {
 *     static const char* zlib[] = {
 *     #ifdef CAPI_TARGET_OS_WIN
 *         "zlib",
 *     #else
 *         "z",
 *     #endif
 *         nullptr
 *     };
 *     static const int versions[] = { 0, ::capi::NoVersion, 1, ::capi::EndVersion };
 *     CAPI_BEGIN_DLL_VER(zlib, versions, ::capi::dso) // you can also use QLibrary or your custom library resolver instead of ::capi::dso
 *     CAPI_DEFINE_ENTRY(const char*, zlibVersion, CAPI_ARG0())
 *     CAPI_DEFINE_ENTRY(const char*, zError, CAPI_ARG1(int))
 *     CAPI_END_DLL()
 *     CAPI_DEFINE_DLL
 *     CAPI_DEFINE(const char*, zlibVersion, CAPI_ARG0())
 *     CAPI_DEFINE(const char*, zError, CAPI_ARG1(int))
 *     } //namespace zlib
 *
 * >test.cpp (dynamically loaded symbols, not link to zlib):
 *
 *     //#define ZLIB_CAPI_NS // namespace style
 *     //#define CAPI_LINK_ZLIB // direct linkt to zlib. add -lz is required
 *     #include <stdio.h>
 *     #include "zlib_api.h"
 *
 *     class test_zlib_api
 *     #ifndef ZLIB_CAPI_NS
 *                 : protected zlib::api // will unload library in dtor
 *     #endif
 *     {
 *     public:
 *         void test_version() {
 *             printf("zlib version: %s\n", zlibVersion());
 *         }
 *         void test_zError(int e) {
 *             printf("zlib error: %d => %s\n", e, zError(e));
 *         }
 *     };
 *
 *     int main(int, char **)
 *     {
 *         test_zlib_api tt;
 *         printf("capi zlib test\n");
 *         tt.test_version();
 *         tt.test_version();
 *         tt.test_zError(1);
 *         return 0;
 *     }
 *
 * ### 3 Styles
 *
 * The same code support 3 styles with only 1 line change in you code (test.cpp above)! You can switch class style and namespace without rebuild api implementation object(zlib_api.cpp).
 *
 * - **Class style(the default)**
 *
 *   All the functions you call are from class `zlib:api` and your class must be a subclass of it.
 *
 * - **Namespace style**
 *
 *   All the functions you call are from namesoace `zlib:capi`. Must add `#define ZLIB_CAPI_NS` before `#include "zlib_api.h"`.
 *
 *   It's easier to use than class style. It is not the default style because if the library is loaded and unloaded multiple times, symbol addresses from first load(wrong!) are used in current implementation.
 *
 * - **Direct link style**
 *
 *   The original functions are called. Must add `#define CAPI_LINK_ZLIB` before `#include "zlib_api.h"`, add `-DCAPI_LINK_ZLIB` to rebuild zlib_api.cpp add `-lz` flags to the compiler
 *
 * ### Lazy Resolve
 *
 * The symbol is resolved at the first call. You can add `#define CAPI_IS_LAZY_RESOLVE 0` in zlib_api.cpp before `#include "QtAV_capi.h"` to resolve all symbols as soon as the library is loaded.
 *
 * ### Auto Code Generation
 *
 * There is a tool to help you generate header and source: https://github.com/wang-bin/mkapi
 *
 * The tool is based on clang 3.4.
 *
 * All you need to do is simply run the tool and use the generated files in your project, maybe with a few modifications.
 *
 * Run `make` to build the tool then run `./mkapi.sh -name zlib zlib.h -I` to generate zlib_api.h and zlib_api.cpp.
 *
 */

#ifndef QTAV_CAPI_H
#define QTAV_CAPI_H

// C++ includes

/*!
 * How To Use: (see test/zlib)
 * use the header and source from code gerenrated from: https://github.com/wang-bin/mkapi
 */

#include <cstddef>  // ptrdiff_t
#include <cstdio>
#include <cassert>
#include <string.h>

// Local includes

#include "digikam_debug.h"

#define CAPI_IS(X) (defined CAPI_IS_##X && CAPI_IS_##X)

/*!
 * you can define CAPI_IS_LAZY_RESOLVE 0 before including capi.h.
 * then all symbols will be resolved in constructor.
 * default resolving a symbol at it's first call
 */

#ifndef CAPI_IS_LAZY_RESOLVE
#   define CAPI_IS_LAZY_RESOLVE 1
#endif

namespace capi
{

namespace version
{
    enum
    {
        Major = 0,
        Minor = 6,
        Patch = 0,
        Value = ((Major&0xff) << 16) | ((Minor&0xff) << 8) | (Patch&0xff)
    };

    static const char name[] = { Major + '0', '.', Minor + '0', '.', Patch + '0', 0 };

} // namespace version

// set lib name with version

enum
{
    NoVersion  = -1, ///< library name without major version, for example libz.so
    EndVersion = -2
};

/********************************** The following code is only used in .cpp **************************************************/

/*!
  * - Library names:
  *  static const char* zlib[] = {
  *  #ifdef CAPI_TARGET_OS_WIN
  *    "zlib",
  *  #else
  *    "z",
  *  #endif
  *    /usr/local/lib/libmyz.so, // absolute path is ok
  *    nullptr};
  *  CAPI_BEGIN_DLL(zlib, ::capi::dso) // the 2nd parameter is a dynamic shared object loader class. \sa dll_helper class
  *  ...
  *
  * - Multiple library versions
  *  An example to open libz.so, libz.so.1, libz.so.0 on unix
  *  static const int ver[] = { ::capi::NoVersion, 1, 0, ::capi::EndVersion };
  *  CAPI_BEGIN_DLL_VER(zlib, ver, ::capi::dso)
  *  ...
  */

class dso
{
private:

    void* handle         = nullptr;
    char  full_name[256] = { 0 };

    dso(const dso&);
    dso& operator=(const dso&);

public:

    dso()
        : handle(nullptr)
    {
    }

    virtual ~dso()
    {
        unload();
    }

    inline void setFileName(const char* name);
    inline void setFileNameAndVersion(const char* name, int ver);
    inline bool load();
    inline bool unload();

    bool isLoaded() const
    {
        return !!handle;
    }

    virtual void* resolve(const char* symbol)
    {
        return resolve(symbol, true);
    }

protected:

    inline void* resolve(const char* sym, bool try_);
};

} // namespace capi

/**
 * DLL_CLASS is a library loader and symbols resolver class. Must implement api like capi::dso (the same function
 * name and return type, but string parameter type can be different):
 * unload() must support ref count. i.e. do unload if no one is using the real library
 * Currently you can use ::capi::dso and QLibrary for DLL_CLASS. You can also use your own library resolver
 */

#define CAPI_BEGIN_DLL(names, DLL_CLASS)                                    \
    class api_dll : public ::capi::internal::dll_helper<DLL_CLASS>          \
    {                                                                       \
        public: api_dll()                                                   \
            : ::capi::internal::dll_helper<DLL_CLASS>(names)                \
    CAPI_DLL_BODY_DEFINE

#define CAPI_BEGIN_DLL_VER(names, versions, DLL_CLASS)                      \
    class api_dll : public ::capi::internal::dll_helper<DLL_CLASS>          \
    {                                                                       \
        public:                                                             \
            api_dll()                                                       \
                : ::capi::internal::dll_helper<DLL_CLASS>(names, versions)  \
     CAPI_DLL_BODY_DEFINE

#if CAPI_IS(LAZY_RESOLVE)
#   define CAPI_END_DLL() } api_t; api_t api; };
#else
#   define CAPI_END_DLL() };
#endif

#define CAPI_DEFINE_DLL api::api():dll(new api_dll()){} \
    api::~api(){delete dll;}                            \
    bool api::loaded() const { return dll->isLoaded();} \
    namespace capi {                                    \
        static api_dll* dll = nullptr;                  \
        bool loaded() {                                 \
            if (!dll) dll = new api_dll();              \
            return dll->isLoaded();                     \
        }                                               \
    }

/*!
 * N: number of arguments
 * R: return type
 * name: api name
 * ...: api arguments with only types, wrapped by CAPI_ARGn, n=0,1,2,...13
 * The symbol of the api is "name". Otherwise, use CAPI_DEFINE instead.
 * example:
 * 1. const char* zlibVersion()
 *    CAPI_DEFINE(const char* zlibVersion, CAPI_ARG0())
 * 2. const char* zError(int) // get error string from zlib error code
 *    CAPI_DEFINE(const char*, zError, CAPI_ARG1(int))
 */

#if CAPI_IS(LAZY_RESOLVE)
#   define CAPI_DEFINE(R, name, ...)            EXPAND(CAPI_DEFINE2_X(R, name, name, __VA_ARGS__)) /* not ##__VA_ARGS__ !*/
#   define CAPI_DEFINE_ENTRY(R, name, ...)      EXPAND(CAPI_DEFINE_ENTRY_X(R, name, name, __VA_ARGS__))
#   define CAPI_DEFINE_M_ENTRY(R, M, name, ...) EXPAND(CAPI_DEFINE_M_ENTRY_X(R, M, name, name, __VA_ARGS__))
#else
#   define CAPI_DEFINE(R, name, ...)            EXPAND(CAPI_DEFINE_X(R, name, __VA_ARGS__)) /* not ##__VA_ARGS__ !*/
#   define CAPI_DEFINE_ENTRY(R, name, ...)      EXPAND(CAPI_DEFINE_RESOLVER_X(R, name, name, __VA_ARGS__))
#   define CAPI_DEFINE_M_ENTRY(R, M, name, ...) EXPAND(CAPI_DEFINE_M_RESOLVER_X(R, M, name, name, __VA_ARGS__))
#endif

//EXPAND(CAPI_DEFINE##N(R, name, #name, __VA_ARGS__))

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/************The followings are used internally**********/

#if CAPI_IS(LAZY_RESOLVE)
#   define CAPI_DLL_BODY_DEFINE { memset(&api, 0, sizeof(api));} typedef struct capi_internal_struct_ {
#else
#   define CAPI_DLL_BODY_DEFINE { CAPI_DBG_RESOLVE("capi resolved dll symbols...");}
#endif

#define CAPI_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V)         \
    R api::name ARG_T_V {                                       \
        CAPI_DBG_CALL(" ");                                     \
        assert(dll && dll->isLoaded() && "dll is not loaded");  \
        return dll->name ARG_V;                                 \
    }

#define CAPI_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V)               \
    R api::name ARG_T_V {                                                   \
        CAPI_DBG_CALL(" ");                                                 \
        assert(dll && dll->isLoaded() && "dll is not loaded");              \
        if (!dll->api.name) {                                               \
            dll->api.name = (api_dll::api_t::name##_t)dll->resolve(#sym);   \
            CAPI_DBG_RESOLVE("dll::api_t::" #name ": @%p", dll->api.name);  \
        }                                                                   \
        assert(dll->api.name && "failed to resolve " #R #sym #ARG_T_V);     \
        return dll->api.name ARG_V;                                         \
    }

/*
 * TODO: choose 1 of below
 * - use CAPI_LINKAGE and remove CAPI_DEFINE_M_ENTRY_X & CAPI_DEFINE_M_RESOLVER_X
 * - also pass a linkage parameter to CAPI_NS_DEFINE_T_V & CAPI_NS_DEFINE2_T_V
 */

#ifndef CAPI_LINKAGE
#   define CAPI_LINKAGE
#endif

#define CAPI_NS_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V)      \
    namespace capi {                                            \
    using capi::dll;                                            \
    R CAPI_LINKAGE name ARG_T_V {                               \
        CAPI_DBG_CALL(" ");                                     \
        if (!dll) dll = new api_dll();                          \
        assert(dll && dll->isLoaded() && "dll is not loaded");  \
        return dll->name ARG_V;                                 \
    } }

#define CAPI_NS_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V)            \
    namespace capi {                                                        \
    using capi::dll;                                                        \
    R CAPI_LINKAGE name ARG_T_V {                                           \
        CAPI_DBG_CALL(" ");                                                 \
        if (!dll) dll = new api_dll();                                      \
        assert(dll && dll->isLoaded() && "dll is not loaded");              \
        if (!dll->api.name) {                                               \
            dll->api.name = (api_dll::api_t::name##_t)dll->resolve(#sym);   \
            CAPI_DBG_RESOLVE("dll::api_t::" #name ": @%p", dll->api.name);  \
        }                                                                   \
        assert(dll->api.name && "failed to resolve " #R #sym #ARG_T_V);     \
        return dll->api.name ARG_V;                                         \
    } }

// nested class can not call non-static members outside the class, so hack the address here
// need -Wno-invalid-offsetof

#define CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, ARG_T, ARG_T_V, ARG_V)                                              \
    public:                                                                                                             \
        typedef R (M *name##_t) ARG_T;                                                                                  \
        name##_t name;                                                                                                  \
    private:                                                                                                            \
        struct name##_resolver_t {                                                                                      \
            name##_resolver_t() {                                                                                       \
                const ptrdiff_t diff = ptrdiff_t(&((api_dll*)0)->name##_resolver) - ptrdiff_t(&((api_dll*)0)->name);    \
                name##_t *p = (name##_t*)((char*)this - diff);                                                          \
                api_dll* dll = (api_dll*)((char*)this - ((ptrdiff_t)(&((api_dll*)0)->name##_resolver)));                \
                if (!dll->isLoaded()) {                                                                                 \
                    CAPI_WARN_LOAD("dll not loaded");                                                                   \
                    *p = nullptr;                                                                                          \
                    return;                                                                                             \
                }                                                                                                       \
                *p = (name##_t)dll->resolve(#sym);                                                                      \
                if (*p) { CAPI_DBG_RESOLVE("dll::" #name ": @%p", *p); }                                                \
                else { CAPI_WARN_RESOLVE("capi resolve error '" #name "'"); }                                           \
            }                                                                                                           \
        } name##_resolver;

#if defined(__GNUC__)
#   define CAPI_FUNC_INFO __PRETTY_FUNCTION__
#elif defined(_MSC_VER)                                 // krazy:exclude=cpp
#   define CAPI_FUNC_INFO __FUNCSIG__
#else
#   define CAPI_FUNC_INFO __FUNCTION__
#endif

#ifdef DEBUG
#   define DEBUG_LOAD
#   define DEBUG_RESOLVE
#   define DEBUG_CALL
#endif

#if defined(DEBUG) || defined(DEBUG_LOAD) || defined(DEBUG_RESOLVE) || defined(DEBUG_CALL)
#   define CAPI_LOG(STDWHERE, fmt, ...) do {qCDebug(DIGIKAM_QTAV_LOG).noquote() << QString::asprintf("[%s] %s@%d: " fmt "\n", __FILE__, CAPI_FUNC_INFO, __LINE__, ##__VA_ARGS__);} while(0);
#else
#   define CAPI_LOG(...)
#endif

#ifdef DEBUG_LOAD
#   define CAPI_DBG_LOAD(...)  EXPAND(CAPI_LOG(stdout, ##__VA_ARGS__))
#   define CAPI_WARN_LOAD(...) EXPAND(CAPI_LOG(stderr, ##__VA_ARGS__))
#else
#   define CAPI_DBG_LOAD(...)
#   define CAPI_WARN_LOAD(...)
#endif

#ifdef DEBUG_RESOLVE
#   define CAPI_DBG_RESOLVE(...)  EXPAND(CAPI_LOG(stdout, ##__VA_ARGS__))
#   define CAPI_WARN_RESOLVE(...) EXPAND(CAPI_LOG(stderr, ##__VA_ARGS__))
#else
#   define CAPI_DBG_RESOLVE(...)
#   define CAPI_WARN_RESOLVE(...)
#endif

#ifdef DEBUG_CALL
#   define CAPI_DBG_CALL(...)  EXPAND(CAPI_LOG(stdout, ##__VA_ARGS__))
#   define CAPI_WARN_CALL(...) EXPAND(CAPI_LOG(stderr, ##__VA_ARGS__))
#else
#   define CAPI_DBG_CALL(...)
#   define CAPI_WARN_CALL(...)
#endif

// fully expand. used by VC. VC will not expand __VA_ARGS__ but treats it as 1 parameter

#define EXPAND(expr) expr   // TODO: rename CAPI_EXPAND
#if defined(WIN64) || defined(_WIN64) || defined(__WIN64__) || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(WINCE) || defined(_WIN32_WCE) // krazy:exclude=cpp
#   define CAPI_TARGET_OS_WIN 1
#   include <windows.h>
#   ifdef WINAPI_FAMILY
#       include <winapifamily.h>
#       if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#           define CAPI_TARGET_OS_WINRT 1
#       endif
#   endif
#endif

#if defined(__APPLE__)               // krazy:exclude=cpp
#   define CAPI_TARGET_OS_MAC 1
#endif

#ifndef CAPI_TARGET_OS_WIN
#   include <dlfcn.h>
#endif

namespace capi
{

namespace internal
{

// the following code is for the case DLL=QLibrary + QT_NO_CAST_FROM_ASCII
// you can add a new qstr_wrap like class and a specialization of dso_trait to support a new string type before/after include "QtAV_capi.h"

struct qstr_wrap
{
    static const char* fromLatin1(const char* s) { return s; }
};

template<class T> struct dso_trait
{
    typedef const char* str_t;
    typedef qstr_wrap   qstr_t;
};

//can not explicit specialization of 'trait' in class scope

#if defined(QLIBRARY_H) && defined(QT_CORE_LIB)

template<> struct dso_trait<QLibrary>
{
    typedef QString str_t;
    typedef QString qstr_t;
};

#endif

// base ctor dll_helper("name")=>derived members in decl order(resolvers)=>derived ctor

static const int kDefaultVersions[] = { ::capi::NoVersion, ::capi::EndVersion };
template <class DLL> class dll_helper
{
    // no CAPI_EXPORT required

    DLL m_lib;

    typename dso_trait<DLL>::str_t strType(const char* s)
    {
        return dso_trait<DLL>::qstr_t::fromLatin1(s);
    }

public:

    explicit dll_helper(const char* names[], const int versions[] = kDefaultVersions)
    {
        static bool is_1st = true;

        if (is_1st)
        {
            is_1st = false;

            qCDebug(DIGIKAM_QTAV_LOG).noquote()
                << QString::asprintf("capi::version: %s", ::capi::version::name);
        }

        for (int i = 0 ; names[i] ; ++i)
        {
            for (int j = 0 ; versions[j] != ::capi::EndVersion ; ++j)
            {
                if (versions[j] == ::capi::NoVersion)
                    m_lib.setFileName(strType(names[i]));
                else
                    m_lib.setFileNameAndVersion(strType(names[i]), versions[j]);

                if (m_lib.load())
                {
                    CAPI_DBG_LOAD("capi loaded {library name: %s, version: %d}", names[i], versions[j]);

                    return;
                }

                CAPI_WARN_LOAD("capi can not load {library name: %s, version %d}", names[i], versions[j]);
            }
        }
    }

    virtual ~dll_helper()
    {
        m_lib.unload();
    }

    bool isLoaded() const
    {
        return m_lib.isLoaded();
    }

    void* resolve(const char* symbol)
    {
        return (void*)m_lib.resolve(symbol);
    }
};

#ifdef CAPI_TARGET_OS_WIN

    static const char kPre[] = "";
    static const char kExt[] = ".dll";

#else

    static const char kPre[] = "lib";

#   ifdef CAPI_TARGET_OS_MAC

    static const char kExt[] = ".dylib";

#   else

    static const char kExt[] = ".so";

#   endif

#endif

} // namespace internal

#ifdef CAPI_TARGET_OS_WIN
#   define CAPI_SNPRINTF  _snprintf
#   define CAPI_SNWPRINTF _snwprintf
#else
#   define CAPI_SNPRINTF  snprintf
#   define CAPI_SNWPRINTF snwprintf
#endif

void dso::setFileName(const char* name)
{
    CAPI_DBG_LOAD("dso.setFileName(\"%s\")", name);

    if (name[0] == '/')
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s", name);
    else
        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s%s", internal::kPre, name, internal::kExt);
}

void dso::setFileNameAndVersion(const char* name, int ver)
{
    CAPI_DBG_LOAD("dso.setFileNameAndVersion(\"%s\", %d)", name, ver);

    if (ver >= 0)
    {

#if defined(CAPI_TARGET_OS_WIN) // ignore version on win. xxx-V.dll?

        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s%s",    internal::kPre, name, internal::kExt);

#elif defined(CAPI_TARGET_OS_MAC)

        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s.%d%s", internal::kPre, name, ver, internal::kExt);

#else

        CAPI_SNPRINTF(full_name, sizeof(full_name), "%s%s%s.%d", internal::kPre, name, internal::kExt, ver);

#endif

    }
    else
    {
        setFileName(name);
    }
}

bool dso::load()
{
    CAPI_DBG_LOAD("dso.load: %s", full_name);

#ifdef CAPI_TARGET_OS_WIN
#   ifdef CAPI_TARGET_OS_WINRT

    wchar_t wname[sizeof(full_name)];
    CAPI_SNWPRINTF(wname, sizeof(wname), L"%s", full_name);
    handle = (void*)::LoadPackagedLibrary(wname, 0);

#   else

    handle = (void*)::LoadLibraryExA(full_name, nullptr, 0); //DONT_RESOLVE_DLL_REFERENCES

#   endif
#else

    handle = ::dlopen(full_name, RTLD_LAZY|RTLD_LOCAL); // try no prefix name if error?

#endif

    return !!handle;
}

bool dso::unload()
{
    if (!isLoaded())
        return true;

#ifdef CAPI_TARGET_OS_WIN

    if (!::FreeLibrary(static_cast<HMODULE>(handle))) // return 0 if error. ref counted
        return false;

#else

    if (::dlclose(handle) != 0)                       // ref counted
        return false;

#endif

    handle = nullptr;                                 // TODO: check ref?

    return true;
}

void* dso::resolve(const char* sym, bool try_)
{
    const char* s = sym;
    char _s[512] = { 0 };                             // old a.out systems add an underscore in front of symbols

    if (!try_)
    {
        // previous has no '_', now has '_'

        CAPI_SNPRINTF(_s, sizeof(_s), "_%s", sym);
        s = _s;
    }

    CAPI_DBG_RESOLVE("dso.resolve(\"%s\", %d)", s, try_);

#ifdef CAPI_TARGET_OS_WIN

    void* ptr = (void*)::GetProcAddress((HMODULE)handle, s);

#else

    void* ptr = ::dlsym(handle, s);

#endif

    if (!ptr && try_)
        return resolve(sym, false);

    return ptr;
}

} // namespace capi

#if defined(_MSC_VER)                                   // krazy:exclude=cpp
#   pragma warning(disable:4098)                        // vc return void
#endif

#ifdef __GNUC__
    // gcc: ((T*)0)->member
    // no #pragma GCC diagnostic push/pop around because code is defined as a macro
#   pragma GCC diagnostic ignored "-Winvalid-offsetof"
#endif

/*!
 * used by .cpp to define the api
 *  e.g. CAPI_DEFINE(cl_int, clGetPlatformIDs, "clGetPlatformIDs", CAPI_ARG3(cl_uint, cl_platform_id*, cl_uint*))
 * sym: symbol of the api in library.
 * Defines both namespace style and class style. User can choose which one to use at runtime by adding a macro before including the header or not: #define SOMELIB_CAPI_NS
 * See test/zlib/zlib_api.h
 */
#define CAPI_DEFINE_X(R, name, ARG_T, ARG_T_V, ARG_V)   \
        CAPI_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V) \
        CAPI_NS_DEFINE_T_V(R, name, ARG_T, ARG_T_V, ARG_V)

/* declare and define the symbol resolvers*/

#define EMPTY_LINKAGE
#define CAPI_DEFINE_RESOLVER_X(R, name, sym, ARG_T, ARG_T_V, ARG_V) CAPI_DEFINE_M_RESOLVER_T_V(R, EMPTY_LINKAGE, name, sym, ARG_T, ARG_T_V, ARG_V)

// api with linkage modifier

#define CAPI_DEFINE_M_RESOLVER_X(R, M, name, sym, ARG_T, ARG_T_V, ARG_V) CAPI_DEFINE_M_RESOLVER_T_V(R, M, name, sym, ARG_T, ARG_T_V, ARG_V)

#define CAPI_DEFINE2_X(R, name, sym, ARG_T, ARG_T_V, ARG_V)   \
        CAPI_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V) \
        CAPI_NS_DEFINE2_T_V(R, name, sym, ARG_T, ARG_T_V, ARG_V)

#define CAPI_DEFINE_ENTRY_X(R, name, sym, ARG_T, ARG_T_V, ARG_V) CAPI_DEFINE_M_ENTRY_X(R, EMPTY_LINKAGE, name, sym, ARG_T, ARG_T_V, ARG_V)

#define CAPI_DEFINE_M_ENTRY_X(R, M, sym, name, ARG_T, ARG_T_V, ARG_V) typedef R (M *name##_t) ARG_T; name##_t name;

#define CAPI_ARG0() (), (), ()
#define CAPI_ARG1(P1) (P1), (P1 p1), (p1)
#define CAPI_ARG2(P1, P2) (P1, P2), (P1 p1, P2 p2), (p1, p2)
#define CAPI_ARG3(P1, P2, P3) (P1, P2, P3), (P1 p1, P2 p2, P3 p3), (p1, p2, p3)
#define CAPI_ARG4(P1, P2, P3, P4) (P1, P2, P3, P4), (P1 p1, P2 p2, P3 p3, P4 p4), (p1, p2, p3, p4)
#define CAPI_ARG5(P1, P2, P3, P4, P5) (P1, P2, P3, P4, P5), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5), (p1, p2, p3, p4, p5)
#define CAPI_ARG6(P1, P2, P3, P4, P5, P6) (P1, P2, P3, P4, P5, P6), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6), (p1, p2, p3, p4, p5, p6)
#define CAPI_ARG7(P1, P2, P3, P4, P5, P6, P7) (P1, P2, P3, P4, P5, P6, P7), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7), (p1, p2, p3, p4, p5, p6, p7)
#define CAPI_ARG8(P1, P2, P3, P4, P5, P6, P7, P8) (P1, P2, P3, P4, P5, P6, P7, P8), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8), (p1, p2, p3, p4, p5, p6, p7, p8)
#define CAPI_ARG9(P1, P2, P3, P4, P5, P6, P7, P8, P9) (P1, P2, P3, P4, P5, P6, P7, P8, P9), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9), (p1, p2, p3, p4, p5, p6, p7, p8, p9)
#define CAPI_ARG10(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10)
#define CAPI_ARG11(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11)
#define CAPI_ARG12(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12)
#define CAPI_ARG13(P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13) (P1, P2, P3, P4, P5, P6, P7, P8, P9, P10, P11, P12, P13), (P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8, P9 p9, P10 p10, P11 p11, P12 p12, P13 p13), (p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13)

#endif // QTAV_CAPI_H
