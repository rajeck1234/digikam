/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-10-31
 * Description : QtAV: Multimedia framework based on Qt and FFmpeg
 *               JNI Modern Interface
 *
 * SPDX-FileCopyrightText: 2012-2022 Wang Bin <wbsecg1 at gmail dot com>
 * SPDX-FileCopyrightText:      2022 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * ============================================================ */

// TODO: reset error before each call, reset exception after each call (Aspect pattern?)

#pragma once

// C++ includes

#include <algorithm>
#include <array>
#include <functional> // std::ref
#include <string>
#include <type_traits>
#include <valarray>
#include <vector>

#include <jni.h>

namespace jmi
{

/*************************** JMI Public APIs Begin ***************************/

// set JavaVM to vm if not null. return previous JavaVM
JavaVM* javaVM(JavaVM *vm = nullptr, jint version = JNI_VERSION_1_4);
JNIEnv *getEnv();
std::string to_string(jstring s, JNIEnv* env = nullptr);
// You have to call DeleteLocalRef() manually for the returned jstring
jstring from_string(const std::string& s, JNIEnv* env = nullptr);

namespace android
{

// current android/app/Application object containing a local ref
jobject application(JNIEnv* env = nullptr);

} // namespace android

struct ClassTag {}; // used by JObject<Tag>. subclasses must define static std::string() name(), with or without "L ;" around
struct MethodTag {}; // used by call() and callStatic(). subclasses must define static const char* name();
struct FieldTag {}; // subclasses must define static const char* name();

namespace detail
{

// TODO: if_XXX, if_XXX_t, if_XXX_v    
template<class Tag>
using if_ClassTag = typename std::enable_if<std::is_base_of<ClassTag, Tag>::value, bool>::type;
template<class Tag>
using  if_MethodTag = typename std::enable_if<std::is_base_of<MethodTag, Tag>::value, bool>::type;
template<class Tag>
using if_FieldTag = typename std::enable_if<std::is_base_of<FieldTag, Tag>::value, bool>::type;
template<class T> struct is_JObject : std::is_base_of<ClassTag, T>::type {}; // TODO: is_detected<signature>
template<class T>
using if_JObject = typename std::enable_if<is_JObject<T>::value, bool>::type;
template<class T>
using if_not_JObject = typename std::enable_if<!is_JObject<T>::value, bool>::type;

}

//template<typename T> // jni primitive types(not all fundamental types), jobject, jstring, ..., JObject, c++ array types
//using if_jni_type = typename std::enable_if<std::is_fundamental<T>::value || if_array<T>::value ||
template<typename T> struct signature;
// string signature_of(T) returns signature of object of type T, signature_of() returns signature of void type
// signature of function ptr
template<typename R, typename... Args> std::string signature_of(R(*)(Args...));
// object must be a class template, thus we can cache class id using static member and call FindClass() only once, and also make it possible to cache method id because method id
template<class CTag>
class JObject : public ClassTag
{
public:
    using Tag = CTag;
    static const std::string className() {
        static std::string s(normalizeClassName(CTag::name()));
        return s;
    }
    static const std::string signature() {return "L" + className() + ";";}

    // construct from an existing jobject. Usually obj is from native jni api containing a local ref, and it's local ref will be deleted if del_localref is true
    JObject(jobject obj = nullptr, bool del_localref = true) {
        JNIEnv *env = getEnv();
        reset(obj, env);
        if (obj && del_localref)
            env->DeleteLocalRef(obj);
    }
    JObject(const JObject &other) { reset(other.id()).setError(other.error()); }
    JObject &operator=(const JObject &other) {  // cppcheck-suppress operatorEqRetRefThis
        if (this == &other)
            return *this;
        return reset(other.id()).setError(other.error());
    }
    JObject(JObject &&other) { // default implementation does not reset other.oid_
        std::swap(oid_, other.oid_);
        std::swap(error_, other.error_);
    }
    JObject &operator=(JObject &&other) { // default implementation does not reset other.oid_
        std::swap(oid_, other.oid_);
        std::swap(error_, other.error_);
        return *this;
    }
    ~JObject() { reset(); }

    operator jobject() const { return oid_;}
    operator jclass() const { return classId();}
    jobject id() const { return oid_; }
    explicit operator bool() const { return !!oid_;}
    std::string error() const {return error_;}
    JObject& reset(jobject obj = nullptr, JNIEnv *env = nullptr);

    template<typename... Args>
    bool create(Args&&... args);

    /* with MethodTag we can avoid calling GetMethodID() in every call()
        struct MyMethod : jmi::MethodTag { static const char* name() { return "myMethod";} };
        return call<T, MyMethod>(args...);
    */
    template<typename T, class MTag, typename... Args,  detail::if_MethodTag<MTag> = true>
    inline T call(Args&&... args) const;
    template<class MTag, typename... Args,  detail::if_MethodTag<MTag> = true>
    inline void call(Args&&... args) const;
    /* with MethodTag we can avoid calling GetStaticMethodID() in every callStatic()
        struct MyStaticMethod : jmi::MethodTag { static const char* name() { return "myStaticMethod";} };
        JObject<CT>::callStatic<R, MyStaticMethod>(args...);
    */
    template<typename T, class MTag, typename... Args,  detail::if_MethodTag<MTag> = true>
    static T callStatic(Args&&... args);
    template<class MTag, typename... Args,  detail::if_MethodTag<MTag> = true>
    static void callStatic(Args&&... args);

    // get/set field and static field
    template<class FTag, typename T, detail::if_FieldTag<FTag> = true>
    T get() const;
    template<class FTag, typename T, detail::if_FieldTag<FTag> = true>
    bool set(T&& v);
    template<class FTag, typename T, detail::if_FieldTag<FTag> = true>
    static T getStatic();
    template<class FTag, typename T, detail::if_FieldTag<FTag> = true>
    static bool setStatic(T&& v);

    // the following call()/callStatic() will always invoke GetMethodID()/GetStaticMethodID()
    template<typename T, typename... Args>
    T call(const std::string& methodName, Args&&... args) const;
    template<typename... Args>
    void call(const std::string& methodName, Args&&... args) const;
    template<typename T, typename... Args>
    static T callStatic(const std::string& name, Args&&... args);
    template<typename... Args>
    static void callStatic(const std::string& name, Args&&... args);

    template<typename T>
    T get(std::string&& fieldName) const;
    template<typename T>
    bool set(std::string&& fieldName, T&& v);
    template<typename T>
    static T getStatic(std::string&& fieldName);
    template<typename T>
    static bool setStatic(std::string&& fieldName, T&& v);

    /*
        Field API
       Field lifetime is bounded to JObject, it does not add object ref, when object is destroyed/reset, accessing Field will fail (TODO: how to avoid crash?)
       jfieldID is cacheable if MayBeFTag is a FieldTag
       Usage:
        auto f = obj.field<int, MyFieldTag>(), obj.field<int>("MyField"), JObject<...>::staticField<string>("MySField");
        auto& sf = JObject<...>::staticField<string, MySFieldTag>();
        f.set(123)/get(), sf.set("test")/get();
        f = 345; int fv = f;
     */
    // F can be supported types: jni primitives(jint, jlong, ... not jobject because we can't know class name) and JObject
    template<typename F, class MayBeFTag, bool isStaticField>
    class Field { // JObject.classId() works in Field?
    public:
        jfieldID id() const { return fid_; }
        operator jfieldID() const { return fid_; }
        operator F() const { return get(); }
        F get() const;
        void set(F&& v);
        Field& operator=(F&& v) {
            set(std::forward<F>(v));
            return *this;
        }
    protected:
        static jfieldID cachedId(jclass cid); // usually cid is used only once
        // oid nullptr: static field
        // it's protected so we can sure cacheable ctor will not be called for uncacheable Field
        explicit Field(jclass cid, jobject oid = nullptr);
        Field(jclass cid, const char* name, jobject oid = nullptr);

        union {
            jobject oid_;
            jclass cid_;
        };
        jfieldID fid_ = nullptr;
        friend class JObject<CTag>;
    };
    template<class FTag, typename T, detail::if_FieldTag<FTag> = true>
    auto field() const->Field<T, FTag, false> {
        return Field<T, FTag, false>(classId(), oid_);
    }
    template<typename T>
    auto field(std::string&& name) const->Field<T, void, false> {
        return Field<T, void, false>(classId(), name.c_str(), oid_);
    }
    template<class FTag, typename T, detail::if_FieldTag<FTag> = true>
    static auto staticField()->Field<T, FTag, true>& { // cacheable and static java storage, so returning ref is better
        static Field<T, FTag, true> f(classId());
        return f;
    }
    template<typename T>
    static auto staticField(std::string&& name)->Field<T, void, true> {
        return Field<T, void, true>(classId(), name.c_str());
    }
private:
    static jclass classId(JNIEnv* env = nullptr);
    JObject& setError(std::string&& s) const {
        error_ = std::move(s);
        return *const_cast<JObject*>(this);
    }
    static std::string normalizeClassName(const std::string& name) {
        std::string s = name;
        if (s[0] == 'L' && s.back() == ';')
            s = s.substr(1, s.size()-2);
        replace(s.begin(), s.end(), '.', '/');
        return s;
    }

    jobject oid_ = nullptr;
    mutable std::string error_;
};
/*************************** JMI Public APIs End ***************************/

/*************************** Below is JMI implementation and internal APIs***************************/
//template<class CTag> inline std::string signature_of(const JObject<CTag>& t) { return t.signature();} // won't work if JObject subclass inherits JObject<...>
template<class T, detail::if_JObject<T> = true>
inline std::string signature_of(const T& t) { return t.signature();}

//signature_of_args<decltype(Args)...>::value, template<typename ...A> struct signature_of_args?

template<> struct signature<jboolean> { static const char value = 'Z';};
template<> struct signature<jbyte> { static const char value = 'B';};
template<> struct signature<jchar> { static const char value = 'C';};
template<> struct signature<jshort> { static const char value = 'S';};
template<> struct signature<jlong> { static const char value = 'J';};
template<> struct signature<jint> { static const char value = 'I';};
template<> struct signature<jfloat> { static const char value = 'F';};
template<> struct signature<jdouble> { static const char value = 'D';};
// "L...;" is used in method parameter
template<> struct signature<std::string> { constexpr static const char* value = "Ljava/lang/String;";};
template<> struct signature<char*> { constexpr static const char* value = "Ljava/lang/String;";};

// T* and T(&)[N] are treated as the same. use enable_if to select 1 of them. The function parameter is (const T&), so the default implemention of signature_of(const T&) must check is_pointer too.
template<typename T> using if_pointer = typename std::enable_if<std::is_pointer<T>::value, bool>::type;
template<typename T> using if_not_pointer = typename std::enable_if<!std::is_pointer<T>::value, bool>::type;
template<typename T> using if_array = typename std::enable_if<std::is_array<T>::value, bool>::type;
template<typename T, if_not_pointer<T> = true, detail::if_not_JObject<T> = true>
inline std::string signature_of(const T&) {
    return {signature<T>::value}; // initializer supports both char and char*
}
inline std::string signature_of(const char*) { return "Ljava/lang/String;";}
inline std::string signature_of(char*) { return "Ljava/lang/String;";}
inline std::string signature_of() { return {'V'};}
// for base types, {'[', signature<T>::value};

template<typename T, std::size_t N>
inline std::string signature_of(const std::array<T, N>&) {
    static const auto s = std::string({'['}).append({signature_of(T())});
    return s;
}

template<typename T, if_pointer<T> = true>
inline std::string signature_of(const T&) { return {signature<jlong>::value};}
template<typename T, std::size_t N>
inline std::string signature_of(const T(&)[N]) { 
    static const auto s = std::string({'['}).append({signature_of(T())});
    return s;
}

template<template<typename, class...> class C, typename T, class... Args> struct is_jarray;
template<typename T, class... Args> struct is_jarray<std::vector, T, Args...> : public std::true_type {};
template<typename T, class... Args> struct is_jarray<std::valarray, T, Args...> : public std::true_type {};
// exclude std::string but not all basic_string because no corresponding java type for wstring
template<class... Args> struct is_jarray<std::basic_string, char, Args...> : public std::false_type {};
template<template<typename, class...> class C, typename T, class... Args> using if_jarray = typename std::enable_if<is_jarray<C, T, Args...>::value, bool>::type;
template<template<typename, class...> class C, typename T, class... Args, if_jarray<C, T, Args...> = true>
inline std::string signature_of(const C<T, Args...>&) {
    static const std::string s = std::string({'['}).append({signature_of(T())});
    return s;
}

// NOTE: define reference_wrapper at last. assume we only use reference_wrapper<...>, no container<reference_wrapper<...>>
template<typename T>
inline std::string signature_of(const std::reference_wrapper<T>&) {
    static const std::string s = signature_of(T()); //TODO: no construct
    return s;
}
template<typename T, std::size_t N>
inline std::string signature_of(const std::reference_wrapper<T[N]>&) {
    static const std::string s = std::string({'['}).append({signature_of(T())});
    //return signature_of<T,N>((T[N]){}); //aggregated initialize. can not use declval?
    return s;
}

namespace detail {
using namespace std;
    static inline bool handle_exception(JNIEnv* env = nullptr) { //'static' function 'handle_exception' declared in header file should be declared 'static inline' [-Wunneeded-internal-declaration]
        if (!env)
            env = getEnv();
        if (!env->ExceptionCheck())
            return false;
        env->ExceptionDescribe();
        env->ExceptionClear();
        return true;
    }

    template<class F>
    class scope_exit_handler {
        F f_;
        bool invoke_;
    public:
        explicit scope_exit_handler(F f) noexcept : f_(move(f)), invoke_(true) {}
        scope_exit_handler(scope_exit_handler&& other) noexcept : f_(move(other.f_)), invoke_(other.invoke_) {
            other.invoke_ = false;
        }
        ~scope_exit_handler() {
            if (invoke_)
                f_();
        }
        scope_exit_handler(const scope_exit_handler&) = delete;
        scope_exit_handler& operator=(const scope_exit_handler&) = delete;
    };
    template<class F>
    scope_exit_handler<F> call_on_exit(const F& f) noexcept {
        return scope_exit_handler<F>(f);
    }
    template<class F>
    scope_exit_handler<F> call_on_exit(F&& f) noexcept {
        return scope_exit_handler<F>(forward<F>(f));
    }


    static inline string make_sig() {return string();}
    template<typename Arg, typename... Args>
    string make_sig(Arg&& arg, Args&&... args) { // initializer_list + (a, b)
        //initializer_list({(s+=signature_of<Args>(args), 0)...});
        return signature_of(forward<Arg>(arg)).append(make_sig(forward<Args>(args)...));
    }

    template<typename... Args>
    string args_signature(Args&&... args) {
        static const string s("(" + make_sig(forward<Args>(args)...) + ")");
        return s;
    }

    template<typename T, if_not_JObject<T> = true>
    jarray make_jarray(JNIEnv *env, const T &element, size_t size); // element is for getting jobject class
    template<class T, if_JObject<T> = true>
    jarray make_jarray(JNIEnv *env, const T &element, size_t size) {
        return env->NewObjectArray(size, jclass(element), nullptr);
    }

    template<typename T, if_not_JObject<T> = true>
    void set_jarray(JNIEnv *env, jarray arr, size_t position, size_t n, const T &elm);
    template<class T, if_JObject<T> = true>
    void set_jarray(JNIEnv *env, jarray arr, size_t position, size_t n, const T &elm) {
        set_jarray(env, arr, position, n, jobject(elm));
    }

    template<typename T>
    jarray to_jarray(JNIEnv* env, const T &c0, size_t N, bool is_ref = false);
    template<typename T, size_t N>
    jarray to_jarray(JNIEnv* env, const T(&c)[N], bool is_ref = false) {
        return to_jarray(env, c[0], N, is_ref);
    }
    template<typename C> // c++ container (vector, valarray, array) to jarray. no if_jarray check (requires overload for both vector like and array like containers) because it's checked by to_jvalue
    jarray to_jarray(JNIEnv* env, const C &c, bool is_ref = false) {
        return to_jarray(env, c[0], c.size(), is_ref);
    }
    // env can be null for base types
    template<typename T> jvalue to_jvalue(const T &obj, JNIEnv* env = nullptr);
    template<typename T> jvalue to_jvalue(T *obj, JNIEnv* env) { return to_jvalue((jlong)obj, env); } // jobject is _jobject*?
    jvalue to_jvalue(const char* obj, JNIEnv* env);// { return to_jvalue(string(obj)); }
    template<template<typename,class...> class C, typename T, class... A, if_jarray<C, T, A...> = true> // if_jarray: exclude std::string, jarray works (copy chars)
    jvalue to_jvalue(const C<T, A...> &c, JNIEnv* env) { return to_jvalue(to_jarray(env, c), env); }
    template<typename T, size_t N> jvalue to_jvalue(const array<T, N> &c, JNIEnv* env) { return to_jvalue(to_jarray(env, c), env); }

    template<typename T> jvalue to_jvalue(const reference_wrapper<T>& t, JNIEnv* env) { return to_jvalue(t.get(), env); } // TODO: no jvalue set
    template<template<typename,class...> class C, typename T, class... A, if_jarray<C, T, A...> = true> // if_jarray: exclude std::string, jarray works (copy chars)
    jvalue to_jvalue(const reference_wrapper<C<T, A...>>& c, JNIEnv* env) { return to_jvalue(to_jarray(env, c.get(), true), env); }
    template<typename T, size_t N> jvalue to_jvalue(const reference_wrapper<T[N]>& c, JNIEnv* env) { return to_jvalue(to_jarray<T,N>(env, c.get(), true), env); }
    template<class CTag>
    jvalue to_jvalue(const JObject<CTag> &obj, JNIEnv* env);
    // T(&)[N]?

    template<typename T, if_not_JObject<T> = true>
    void from_jarray(JNIEnv* env, const jvalue& v, T* t, size_t N);
    template<typename T, if_JObject<T> = true>
    void from_jarray(JNIEnv* env, const jvalue& v, T* t, size_t N) {
        for (size_t i = 0; i < N; ++i) {
            jobject s = env->GetObjectArrayElement(static_cast<jobjectArray>(v.l), i);
            (t + i)->reset(s);
            env->DeleteLocalRef(s);
        }
    }
    // env can be null for base types
    template<typename T> void from_jvalue(JNIEnv* env, const jvalue& v, T &t);
    template<typename T> void from_jvalue(JNIEnv* env, const jvalue& v, T *t, size_t n = 0) { // T* and T(&)[N] is the same
        if (n == 0)
            from_jvalue(env, v, (jlong&)t);
        else
            from_jarray(env, v, t, n);
    }
    template<template<typename,class...> class C, typename T, class... A, if_jarray<C, T, A...> = true> // if_jarray: exclude std::string. jarray works too (copy chars)
    void from_jvalue(JNIEnv* env, const jvalue& v, C<T, A...> &t) { from_jarray(env, v, &t[0], t.size()); }
    template<typename T, size_t N> void from_jvalue(JNIEnv* env, const jvalue& v, array<T, N> &t) { return from_jarray(env, v, t.data(), N); }
    //template<typename T, size_t N> void from_jvalue(JNIEnv* env, const jvalue& v, T(&t)[N]) { return from_jarray(env, v, t, N); }

    template<typename T> struct has_local_ref {
        static const bool value = !is_fundamental<T>::value && !is_pointer<T>::value && !is_JObject<T>::value;
    };
    template<typename T>
    void set_ref_from_jvalue(JNIEnv* env, jvalue* jargs, T) {
        using Tn = typename remove_reference<T>::type;
        if (has_local_ref<Tn>::value)
            env->DeleteLocalRef(jargs->l);
    }
    static inline void set_ref_from_jvalue(JNIEnv* env, jvalue *jargs, const char* s) {
        env->DeleteLocalRef(jargs->l);
    }
    template<typename T>
    void set_ref_from_jvalue(JNIEnv* env, jvalue *jargs, reference_wrapper<T> ref) {
        from_jvalue(env, *jargs, ref.get());
        using Tn = typename remove_reference<T>::type;
        if (has_local_ref<Tn>::value)
            env->DeleteLocalRef(jargs->l);
    }
    static inline void delete_array_local_ref(JNIEnv* env, jarray a, size_t n, bool delete_elements) {
        if (delete_elements) {
            for (jsize i = 0; i < n; ++i) {
                jobject ei = env->GetObjectArrayElement(jobjectArray(a), i);
                if (ei)
                    env->DeleteLocalRef(ei);
            }
        }
        env->DeleteLocalRef(a);
    }
    template<template<typename,class...> class C, typename T, class... A, if_jarray<C, T, A...> = true> // if_jarray: exclude std::string, jarray works (copy chars)
    void set_ref_from_jvalue(JNIEnv* env, jvalue *jargs, reference_wrapper<C<T, A...>> ref) {
        from_jvalue(env, *jargs, ref.get());
        using Tn = typename remove_reference<T>::type;
        delete_array_local_ref(env, static_cast<jarray>(jargs->l), ref.get().size(), has_local_ref<Tn>::value);
    }
    template<typename T, size_t N>
    void set_ref_from_jvalue(JNIEnv* env, jvalue *jargs, reference_wrapper<T[N]> ref) {
        from_jvalue(env, *jargs, ref.get(), N); // assume only T* and T[N]
        delete_array_local_ref(env, static_cast<jarray>(jargs->l), N, has_local_ref<T>::value);
    }
    template<typename T, size_t N>
    void set_ref_from_jvalue(JNIEnv* env, jvalue *jargs, reference_wrapper<array<T, N>> ref) {
        from_jvalue(env, *jargs, &ref.get()[0], N); // assume only T* and T[N]
        delete_array_local_ref(env, static_cast<jarray>(jargs->l), N, has_local_ref<T>::value);
    }

    static inline void ref_args_from_jvalues(JNIEnv* env, jvalue*) {}
    template<typename Arg, typename... Args>
    void ref_args_from_jvalues(JNIEnv* env, jvalue *jargs, Arg& arg, Args&&... args) {
        set_ref_from_jvalue(env, jargs, arg);
        ref_args_from_jvalues(env, jargs + 1, forward<Args>(args)...);
    }

    template<typename T, if_not_JObject<T> = true>
    T call_method(JNIEnv *env, jobject oid, jmethodID mid, jvalue *args);
    template<class T, if_JObject<T> = true>
    T call_method(JNIEnv *env, jobject oid, jmethodID mid, jvalue *args) {
        T t;
        t.reset(call_method<jobject>(env, oid, mid, args), env);
        return t;
    }

    template<typename T, typename... Args>
    T call_method_set_ref(JNIEnv *env, jobject oid, jmethodID mid, jvalue *jargs, Args&&... args) {
        auto setter = call_on_exit([=]{
            ref_args_from_jvalues(env, jargs, args...);
        });
       return call_method<T>(env, oid, mid, jargs);
    }

    template<typename T, if_not_JObject<T> = true>
    T call_static_method(JNIEnv *env, jclass classId, jmethodID methodId, jvalue *args);
    template<class T, if_JObject<T> = true>
    T call_static_method(JNIEnv *env, jclass cid, jmethodID mid, jvalue *args) {
        T t;
        t.reset(call_static_method<jobject>(env, cid, mid, args), env);
        return t;
    }
    template<typename T, typename... Args>
    T call_static_method_set_ref(JNIEnv *env, jclass cid, jmethodID mid, jvalue *jargs, Args&&... args) {
        auto setter = call_on_exit([=]{
            ref_args_from_jvalues(env, jargs, args...);
        });
        return call_static_method<T>(env, cid, mid, jargs);
    }

    template<typename T, typename... Args>
    T call_with_methodID(jobject oid, jclass cid, jmethodID* pmid, function<void(string&& err)> err_cb, const string& signature, const char* name, Args&&... args) {
        if (err_cb)
            err_cb(string());
        if (!cid)
            return T();
        if (!oid) {
            if (err_cb)
                err_cb("Invalid object instance");
            return T();
        }
        JNIEnv *env = getEnv();
        auto checker = call_on_exit([=]{
            if (handle_exception(env)) {
                if (err_cb)
                    err_cb(string("Failed to call method '") + name + "' with signature '" + signature + "'.");
            }
        });
        jmethodID mid = nullptr;
        if (pmid)
            mid = *pmid;
        if (!mid) {
            mid = env->GetMethodID(cid, name, signature.c_str());
            if (pmid)
                *pmid = mid;
        }
        if (!mid || env->ExceptionCheck())
            return T();
        return call_method_set_ref<T>(env, oid, mid, const_cast<jvalue*>(initializer_list<jvalue>({to_jvalue(forward<Args>(args), env)...}).begin()), forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T call_static_with_methodID(jclass cid, jmethodID* pmid, std::function<void(std::string&& err)> err_cb, const std::string& signature, const char* name, Args&&... args) {
        if (err_cb)
            err_cb(string());
        if (!cid)
            return T();
        JNIEnv *env = getEnv();
        auto checker = call_on_exit([=]{
            if (handle_exception(env)) {
                if (err_cb)
                    err_cb(string("Failed to call static method '") + name + "'' with signature '" + signature + "'.");
            }
        });
        jmethodID mid = nullptr;
        if (pmid)
            mid = *pmid;
        if (!mid) {
            mid = env->GetStaticMethodID(cid, name, signature.c_str());
            if (pmid)
                *pmid = mid;
        }
        if (!mid || env->ExceptionCheck())
            return T();
        return call_static_method_set_ref<T>(env, cid, mid, const_cast<jvalue*>(initializer_list<jvalue>({to_jvalue(forward<Args>(args), env)...}).begin()), forward<Args>(args)...);
    }


    template<typename T>
    jfieldID get_field_id(JNIEnv* env, jclass cid, const char* name, jfieldID* pfid = nullptr) {
        jfieldID fid = nullptr;
        if (pfid)
            fid = *pfid;
        if (!fid) {
            fid = env->GetFieldID(cid, name, signature_of(T()).c_str());
            if (pfid)
                *pfid = fid;
        }
        return fid;
    }
    template<class T, if_not_JObject<T> = true>
    T get_field(JNIEnv* env, jobject oid, jfieldID fid);
    template<class T, if_JObject<T> = true>
    T get_field(JNIEnv* env, jobject oid, jfieldID fid) {
        T t;
        t.reset(env->GetObjectField(oid, fid), env);
        return t;
    }
    template<typename T>
    T get_field(jobject oid, jclass cid, jfieldID* pfid, const char* name) {
        JNIEnv* env = getEnv();
        // TODO: call_on_exit?
        jfieldID fid = get_field_id<T>(env, cid, name, pfid);
        if (!fid)
            return T();
        return get_field<T>(env, oid, fid);
    }
    template<class T>
    void set_field(JNIEnv* env, jobject oid, jfieldID fid, T&& v);
    template<typename T>
    void set_field(jobject oid, jclass cid, jfieldID* pfid, const char* name, T&& v) {
        JNIEnv* env = getEnv();
        // TODO: call_on_exit?
        jfieldID fid = get_field_id<T>(env, cid, name, pfid);
        if (!fid)
            return;
        set_field<T>(env, oid, fid, forward<T>(v));
    }

    template<typename T>
    jfieldID get_static_field_id(JNIEnv* env, jclass cid, const char* name, jfieldID* pfid = nullptr) {
        jfieldID fid = nullptr;
        if (pfid)
            fid = *pfid;
        if (!fid) {
            fid = env->GetStaticFieldID(cid, name, signature_of(T()).c_str());
            if (pfid)
                *pfid = fid;
        }
        return fid;
    }
    template<typename T, if_not_JObject<T> = true>
    T get_static_field(JNIEnv* env, jclass cid, jfieldID fid);
    template<class T, if_JObject<T> = true>
    T get_static_field(JNIEnv* env, jclass cid, jfieldID fid) {
        T t;
        t.reset(env->GetObjectField(cid, fid), env);
        return t;
    }
    template<typename T>
    T get_static_field(jclass cid, jfieldID* pfid, const char* name) {
        JNIEnv* env = getEnv();
        jfieldID fid = get_static_field_id<T>(env, cid, name, pfid);
        if (!fid)
            return T();
        return get_static_field<T>(env, cid, fid);
    }
    template<typename T>
    void set_static_field(JNIEnv* env, jclass cid, jfieldID fid, T&& v);
    template<typename T>
    void set_static_field(jclass cid, jfieldID* pfid, const char* name, T&& v) {
        JNIEnv* env = getEnv();
        jfieldID fid = get_static_field_id<T>(env, cid, name, pfid);
        if (!fid)
            return;
        set_static_field<T>(env, cid, fid, forward<T>(v));
    }
} // namespace detail

template<typename R, typename... Args>
std::string signature_of(R (*)(Args...)) {
    static const auto s(detail::args_signature(std::forward<Args>(Args())...).append(signature_of(R())));
    return s;
}
template<typename... Args>
std::string signature_of(void (*)(Args...)) {
    static const auto s(detail::args_signature(std::forward<Args>(Args())...).append(signature_of()));
    return s;
}

template<class CTag>
JObject<CTag>& JObject<CTag>::reset(jobject obj, JNIEnv *env) {
    error_.clear();
    if (!env) {
        env = getEnv();
        if (!env)
            return setError("Invalid JNIEnv");
    }
    if (oid_)
        env->DeleteGlobalRef(oid_);
    oid_ = nullptr;
    if (obj) {
        oid_ = env->NewGlobalRef(obj);
        //env->DeleteLocalRef(obj); // obj from JObject has no local ref
    }
    return *this;
}

template<class CTag>
template<typename... Args>
bool JObject<CTag>::create(Args&&... args) {
    using namespace std;
    using namespace detail;
    JNIEnv* env = nullptr; // FIXME: why build error if let env be the last parameter of create()?
    //if (!env)
    {
        env = getEnv();
        if (!env) {
            setError("No JNIEnv when creating class '" + className() + "'");
            return false;
        }
    }
    const jclass cid = classId();
    if (!cid) {
        setError("Failed to find class '" + className() + "'");
        return false;
    }
    auto checker = call_on_exit([=]{ handle_exception(env); });
    static const auto s(args_signature(forward<Args>(args)...).append(signature_of())); // void
    static const jmethodID mid = env->GetMethodID(cid, "<init>", s.c_str()); // can be static because class id, signature and arguments combination is unique
    if (!mid) {
        setError(string("Failed to find constructor of '" + className() + "' with signature '" + s + "'."));
        return false;
    }
    const jobject oid = env->NewObjectA(cid, mid, const_cast<jvalue*>(initializer_list<jvalue>({to_jvalue(forward<Args>(args), env)...}).begin())); // ptr0(jv) crash
    if (!oid) {
        setError(string("Failed to call constructor '" + className() + "' with signature '" + s + "'."));
        return false;
    }
    reset(oid, env);
    return !!oid_;
}

template<class CTag>
template<typename T, class MTag, typename... Args,  detail::if_MethodTag<MTag>>
T JObject<CTag>::call(Args&&... args) const {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of(T())); // cppcheck-suppress accessForwarded
    static jmethodID mid = nullptr;
    return call_with_methodID<T>(oid_, classId(), &mid, [this](std::string&& err){ setError(std::move(err));}, s, MTag::name(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}
template<class CTag>
template<class MTag, typename... Args,  detail::if_MethodTag<MTag>>
void JObject<CTag>::call(Args&&... args) const {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of()); // cppcheck-suppress accessForwarded
    static jmethodID mid = nullptr;
    call_with_methodID<void>(oid_, classId(), &mid, [this](std::string&& err){ setError(std::move(err));}, s, MTag::name(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}
template<class CTag>
template<typename T, class MTag, typename... Args,  detail::if_MethodTag<MTag>>
T JObject<CTag>::callStatic(Args&&... args) {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of(T())); // cppcheck-suppress accessForwarded
    static jmethodID mid = nullptr;
    return call_static_with_methodID<T>(classId(), &mid, nullptr, s, MTag::name(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}
template<class CTag>
template<class MTag, typename... Args,  detail::if_MethodTag<MTag>>
void JObject<CTag>::callStatic(Args&&... args) {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of()); // cppcheck-suppress accessForwarded
    static jmethodID mid = nullptr;
    return call_static_with_methodID<void>(classId(), &mid, nullptr, s, MTag::name(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}

template<class CTag>
template<class FTag, typename T, detail::if_FieldTag<FTag>>
T JObject<CTag>::get() const {
    static jfieldID fid = nullptr;
    auto checker = detail::call_on_exit([=]{
        if (detail::handle_exception()) // TODO: check fid
            setError(std::string("Failed to get field '") + FTag::name() + "' with signature '" + signature_of(T()) + "'.");
    });
    return detail::get_field<T>(oid_, classId(), &fid, FTag::name());
}
template<class CTag>
template<class FTag, typename T, detail::if_FieldTag<FTag>>
bool JObject<CTag>::set(T&& v) {
    static jfieldID fid = nullptr;
    auto checker = detail::call_on_exit([=]{
        if (detail::handle_exception())
            setError(std::string("Failed to set field '") + FTag::name() + "' with signature '" + signature_of(T()) + "'.");
    });
    detail::set_field<T>(oid_, classId(), &fid, FTag::name(), std::forward<T>(v));
    return true;
}
template<class CTag>
template<class FTag, typename T, detail::if_FieldTag<FTag>>
T JObject<CTag>::getStatic() {
    static jfieldID fid = nullptr;
    return detail::get_static_field<T>(classId(), &fid, FTag::name());
}
template<class CTag>
template<class FTag, typename T, detail::if_FieldTag<FTag>>
bool JObject<CTag>::setStatic(T&& v) {
    static jfieldID fid = nullptr;
    detail::set_static_field<T>(classId(), &fid, FTag::name(), std::forward<T>(v));
    return true;
}


template<class CTag>
template<typename T, typename... Args>
T JObject<CTag>::call(const std::string &methodName, Args&&... args) const {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of(T())); // cppcheck-suppress accessForwarded
    return call_with_methodID<T>(oid_, classId(), nullptr, [this](std::string&& err){ setError(std::move(err));}, s, methodName.c_str(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}
template<class CTag>
template<typename... Args>
void JObject<CTag>::call(const std::string &methodName, Args&&... args) const {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of()); // cppcheck-suppress accessForwarded
    call_with_methodID<void>(oid_, classId(), nullptr, [this](std::string&& err){ setError(std::move(err));}, s, methodName.c_str(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}
template<class CTag>
template<typename T, typename... Args>
T JObject<CTag>::callStatic(const std::string &name, Args&&... args) {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of(T())); // cppcheck-suppress accessForwarded
    return call_static_with_methodID<T>(classId(), nullptr, nullptr, s, name.c_str(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}
template<class CTag>
template<typename... Args>
void JObject<CTag>::callStatic(const std::string &name, Args&&... args) {
    using namespace detail;
    static const auto s = args_signature(std::forward<Args>(args)...).append(signature_of()); // cppcheck-suppress accessForwarded
    call_static_with_methodID<void>(classId(), nullptr, nullptr, s, name.c_str(), std::forward<Args>(args)...); // cppcheck-suppress accessForwarded
}

template<class CTag>
template<typename T>
T JObject<CTag>::get(std::string&& fieldName) const {
    jfieldID fid = nullptr;
    auto checker = detail::call_on_exit([=]{
        if (detail::handle_exception()) // TODO: check fid
            setError(std::string("Failed to get field '") + fieldName + "' with signature '" + signature_of(T()) + "'.");
    });
    return detail::get_field<T>(oid_, classId(), &fid, fieldName.c_str());
}
template<class CTag>
template<typename T>
bool JObject<CTag>::set(std::string&& fieldName, T&& v) {
    jfieldID fid = nullptr;
    auto checker = detail::call_on_exit([=]{
        if (detail::handle_exception())
            setError(std::string("Failed to set field '") + fieldName + "' with signature '" + signature_of(T()) + "'.");
    });
    detail::set_field<T>(oid_, classId(), &fid, fieldName.c_str(), std::forward<T>(v));
    return true;
}
template<class CTag>
template<typename T>
T JObject<CTag>::getStatic(std::string&& fieldName) {
    jfieldID fid = nullptr;
    return detail::get_static_field<T>(classId(), &fid, fieldName.c_str());
}
template<class CTag>
template<typename T>
bool JObject<CTag>::setStatic(std::string&& fieldName, T&& v) {
    jfieldID fid = nullptr;
    detail::set_static_field<T>(classId(), &fid, fieldName.c_str(), std::forward<T>(v));
    return true;
}

template<class CTag>
template<typename F, class MayBeFTag, bool isStaticField>
F JObject<CTag>::Field<F, MayBeFTag, isStaticField>::get() const
{
    auto checker = detail::call_on_exit([=]{
        detail::handle_exception();
    });
    if (isStaticField)
        return detail::get_static_field<F>(getEnv(), cid_, id());
    return detail::get_field<F>(getEnv(), oid_, id());
}

template<class CTag>
template<typename F, class MayBeFTag, bool isStaticField>
void JObject<CTag>::Field<F, MayBeFTag, isStaticField>::set(F&& v)
{
    auto checker = detail::call_on_exit([=]{
        detail::handle_exception();
    });
    if (isStaticField)
        detail::set_static_field<F>(getEnv(), cid_, id(), std::forward<F>(v));
    else
        detail::set_field<F>(getEnv(), oid_, id(), std::forward<F>(v));
}

template<class CTag>
template<typename F, class MayBeFTag, bool isStaticField>
jfieldID JObject<CTag>::Field<F, MayBeFTag, isStaticField>::cachedId(jclass cid)
{
    static jfieldID fid = nullptr;
    if (!fid) {
        if (isStaticField)
            fid = detail::get_static_field_id<F>(getEnv(), cid, MayBeFTag::name());
        else
            fid = detail::get_field_id<F>(getEnv(), cid, MayBeFTag::name());
    }
    return fid;
}

template<class CTag>
template<typename F, class MayBeFTag, bool isStaticField>
JObject<CTag>::Field<F, MayBeFTag, isStaticField>::Field(jclass cid, jobject oid)
 : oid_(oid):fid(cachedId(cid)) {
    if (isStaticField)
        cid_ = cid;
}

template<class CTag>
template<typename F, class MayBeFTag, bool isStaticField>
JObject<CTag>::Field<F, MayBeFTag, isStaticField>::Field(jclass cid, const char* name, jobject oid)
 : oid_(oid) {
    if (isStaticField) {
        fid_ = detail::get_static_field_id<F>(getEnv(), cid, name);
        cid_ = cid;
    } else {
        fid_ = detail::get_field_id<F>(getEnv(), cid, name);
    }
}


template<class CTag>
jclass JObject<CTag>::classId(JNIEnv* env) {
    static jclass c = nullptr;
    if (!c) {
        if (!env) {
            env = getEnv();
            if (!env)
                return c;
        }
        const jclass cid = (jclass)env->FindClass(className().c_str());
        if (cid) {
            c = (jclass)env->NewGlobalRef(cid); // cache per (c++/java)class class id
            env->DeleteLocalRef(cid); // can not use c
        }
    }
    return c;
}

namespace detail
{

template<typename T>
jarray to_jarray(JNIEnv* env, const T &c0, size_t N, bool is_ref) {
    if (!env) {
        env = getEnv();
        if (!env)
            return nullptr;
    }
    jarray arr = nullptr;
    if (N == 0)
        arr = make_jarray(env, T(), 0);
    else
        arr = make_jarray(env, c0, N);
    if (!is_ref) {
        if (std::is_fundamental<T>::value) {
            set_jarray(env, arr, 0, N, c0);
        } else { // string etc. must convert to jobject
            for (std::size_t i = 0; i < N; ++i)
                set_jarray(env, arr, i, 1, *((&c0)+i));
        }
    }
    return arr;
}
template<class CTag>
jvalue to_jvalue(const JObject<CTag> &obj, JNIEnv* env) {
    return to_jvalue(jobject(obj), env);
}

} // namespace detail

} // namespace jmi
