#include "Application_jni.h"
#include "kwui_jni.h"
#include "api/kwui/Application.h"
#include "api/kwui/ScriptEngine.h"
#include "absl/base/macros.h"
#include "base/log.h"
#include "tools/sk_app/DisplayParams.h"
#include "tools/sk_app/WindowContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"
#include "android/SurfaceAndroid.h"
#include "android/DialogAndroid.h"
#include "absl/base/internal/per_thread_tls.h"
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <string>
#include <memory>
#include <unordered_set>

namespace android {

static std::unordered_set<std::string> create_dialog_cb_set;

enum MessageType {
    kUndefined,
    kActivityCreated,
    kSurfaceChanged,
    kSurfaceDestroyed,
    kSurfaceRedrawNeeded,
    kScrollEvent,
    kShowPressEvent,
    kLongPressEvent,
    kSingleTapConfirmedEvent,
};

struct Message {
    MessageType fType = kUndefined;
    std::string id;
    ANativeWindow* fNativeWindow = nullptr;
    float fDensity = 1.0f;
    float x, y;
    float dx, dy;
    //SkPicture* fPicture = nullptr;
    //WindowSurface** fWindowSurface = nullptr;

    Message() {}
    Message(MessageType t) : fType(t) {}
};

class JApplication;
static JApplication* g_japp = nullptr;
static ABSL_PER_THREAD_TLS_KEYWORD bool gt_main_thread = false;

static ABSL_PER_THREAD_TLS_KEYWORD JNIEnv* jmain_jni_env = nullptr;
static jclass jclass_native = nullptr;
static jmethodID jmethod_create_dialog = nullptr;
static jmethodID jmethod_release_dialog = nullptr;

static int start_logger(const char* app_name);
static std::string string_from_jni(JNIEnv* env, jstring jstr);

class JApplication {
public:
    JApplication(jobject asset_manager, const std::string& entry_js);

    void postMessage(const Message& message) const;
    void readMessage(Message* message) const;
    void release();
    //private:
    static void* pthread_main(void* arg);
    static int message_callback(int fd, int events, void* data);
    // TODO: This has to be static, which is weird now, but fine in a singleton
    // Switch to singleton design or find other way to break out of thread loop
    bool fRunning;

    pthread_t fThread;
    int fPipe[2]; // acts as a Message queue, read from [0] write to [1]
    jobject asset_manager_;
    std::string entry_js_;
};

JApplication::JApplication(jobject asset_manager, const std::string& entry_js)
    : asset_manager_(asset_manager), entry_js_(entry_js)
{
    pipe(fPipe);
    //LOG(INFO) << "JApplication::JApplication() " << fPipe[0] << "/" << fPipe[1];
    fRunning = true;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

void JApplication::postMessage(const Message& message) const
{
    ssize_t ret = write(fPipe[1], &message, sizeof(message));
}

void JApplication::readMessage(Message* message) const
{
    ssize_t ret = read(fPipe[0], message, sizeof(Message));
}

void JApplication::release() {
    fRunning = false;
    pthread_join(fThread, nullptr);
}

int JApplication::message_callback(int /* fd */, int /* events */, void* data) {
    auto app = (JApplication*)data;
    Message message;
    app->readMessage(&message);
    // get target surface from Message

    switch (message.fType) {
    case kActivityCreated: {
        DialogAndroid::handleActivityCreated();
        break;
    }
    case kSurfaceChanged: {
        auto dlg = DialogAndroid::findDialogById(message.id);
        if (dlg) {
            dlg->handleSurfaceChanged(message.fNativeWindow, message.fDensity);
        } else {
            LOG(ERROR) << "kSurfaceChanged: dialog " << message.id << " not found.";
        }
        break;
    }
    case kSurfaceDestroyed: {
        auto dlg = DialogAndroid::findDialogById(message.id);
        if (dlg) {
            dlg->handleSurfaceDestroyed();
        } else {
            LOG(ERROR) << "kSurfaceDestroyed: dialog " << message.id << " not found.";
        }
        break;
    }
    case kSurfaceRedrawNeeded: {
        auto dlg = DialogAndroid::findDialogById(message.id);
        if (dlg) {
            dlg->handleSurfaceRedrawNeeded();
        } else {
            LOG(ERROR) << "kSurfaceDestroyed: dialog " << message.id << " not found.";
        }
        break;
    }
    case kScrollEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleScrollEvent(message.x, message.y, message.dx, message.dy);
        }
        break;
    }
    case kShowPressEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleShowPressEvent(message.x, message.y);
        }
        break;
    }
    case kLongPressEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleLongPressEvent(message.x, message.y);
        }
        break;
    }
    case kSingleTapConfirmedEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleSingleTapConfirmedEvent(message.x, message.y);
        }
        break;
    }
    default: {
        // do nothing
    }
    }

    return 1;  // continue receiving callbacks
}


void* JApplication::pthread_main(void* arg) {
    gt_main_thread = true;
    start_logger("script");
    auto me = (JApplication*)arg;
    // Looper setup
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, me->fPipe[0], 1, ALOOPER_EVENT_INPUT, nullptr, nullptr);
    jmain_jni_env = nullptr;
    kwui_java_vm->AttachCurrentThread(&jmain_jni_env, nullptr);

    kwui::Application app(jmain_jni_env, me->asset_manager_);
    //LOG(INFO) << "JApplication::pthread_main(): entry_js=[" << me->entry_js_ << "]";
    if (!me->entry_js_.empty()) {
        kwui::ScriptEngine::get()->loadFile(me->entry_js_.c_str());
    }
    while (me->fRunning) {
        const int ident = ALooper_pollOnce(0, nullptr, nullptr, nullptr);

        //if (ident != ALOOPER_POLL_TIMEOUT) {
            //SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        //LOG(INFO) << "ALooper_pollOnce return " << ident;
        if (ident == 1) {
            message_callback(0, 0, me);
        }
        //}
    }
    return nullptr;
}

static jlong Application_Create(JNIEnv* env, jclass, jobject asset_manager, jstring entry_js)
{
    if (g_japp) {
        g_japp->postMessage(Message(kActivityCreated));
        return reinterpret_cast<jlong>(g_japp);
    }
    auto asset_manager_ref = env->NewGlobalRef(asset_manager);
    const char* u8_js_path = env->GetStringUTFChars(entry_js, nullptr);
    std::string js_entry_str(u8_js_path);
    env->ReleaseStringUTFChars(entry_js, u8_js_path);

    g_japp = new JApplication(asset_manager_ref, js_entry_str);
    return reinterpret_cast<jlong>(g_japp);
}

static void Application_Destroy(JNIEnv* env, jobject, jlong ptr)
{
    auto japp = reinterpret_cast<JApplication*>(ptr);
    japp->release();
    delete japp;
    g_japp = nullptr;
}

static void Application_SurfaceChanged(JNIEnv* env, jobject, jlong ptr, jstring jid, jobject surface,
    jint /*format*/, jint /*width*/, jint /*height*/, jfloat density)
{
    LOG(INFO) << "Application_SurfaceChanged";
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceChanged);
    msg.id = string_from_jni(env, jid);
    msg.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    msg.fDensity = density;
    me->postMessage(msg);
}

static void Application_SurfaceDestroyed(JNIEnv* env, jobject, jlong ptr, jstring jid)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceDestroyed);
    msg.id = string_from_jni(env, jid);
    me->postMessage(msg);
}
static void Application_SurfaceRedrawNeeded(JNIEnv* env, jobject, jlong ptr, jstring jid)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceRedrawNeeded);
    msg.id = string_from_jni(env, jid);
    me->postMessage(msg);
}
static void Application_HandleTouchEvent(JNIEnv* env, jobject, jlong ptr, jint action, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    // LOG(INFO) << "touch event: action " << action << " " << x << "," << y;
    //me->postMessage(Message(kSurfaceRedrawNeeded));
}

static void Application_HandleScrollEvent(JNIEnv* env, jobject, jlong ptr, jstring jid, jfloat x, jfloat y, jfloat dx, jfloat dy)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "scroll event: " << dx << "," << dy;
    Message msg(kScrollEvent);
    msg.id = string_from_jni(env, jid);
    msg.x = x;
    msg.y = y;
    msg.dx = dx;
    msg.dy = dy;
    me->postMessage(msg);
}
static void Application_HandleShowPressEvent(JNIEnv* env, jobject, jlong ptr, jstring jid, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "show press event: " << x << "," << y;
    Message msg(kShowPressEvent);
    msg.id = string_from_jni(env, jid);
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_HandleLongPressEvent(JNIEnv* env, jobject, jlong ptr, jstring jid, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "long press event: " << x << "," << y;
    Message msg(kLongPressEvent);
    msg.id = string_from_jni(env, jid);
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_HandleSingleTapConfirmedEvent(JNIEnv* env, jobject, jlong ptr, jstring jid, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "single tap confirmed event: " << x << "," << y;
    Message msg(kSingleTapConfirmedEvent);
    msg.id = string_from_jni(env, jid);
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}

int kwui_jni_register_Application(JNIEnv* env)
{
    const auto clazz = env->FindClass("prj/kwui/Native");
    if (clazz) {
        jclass_native = reinterpret_cast<jclass>(env->NewGlobalRef(reinterpret_cast<jobject>(clazz)));
        jmethod_create_dialog = env->GetStaticMethodID(jclass_native, "jCreateDialog", "(Ljava/lang/String;)V");
        jmethod_release_dialog = env->GetStaticMethodID(jclass_native, "jReleaseDialog", "(Ljava/lang/String;)V");
    }

    static const JNINativeMethod methods[] = {
        {"nCreate" , "(Landroid/content/res/AssetManager;Ljava/lang/String;)J", reinterpret_cast<void*>(Application_Create)},
        {"nDestroy", "(J)V", reinterpret_cast<void*>(Application_Destroy)},
        {"nSurfaceChanged", "(JLjava/lang/String;Landroid/view/Surface;IIIF)V", reinterpret_cast<void*>(Application_SurfaceChanged)},
        {"nSurfaceDestroyed", "(JLjava/lang/String;)V", reinterpret_cast<void*>(Application_SurfaceDestroyed)},
        {"nSurfaceRedrawNeeded", "(JLjava/lang/String;)V", reinterpret_cast<void*>(Application_SurfaceRedrawNeeded)},
        {"nHandleTouchEvent", "(JLjava/lang/String;IFF)V", reinterpret_cast<void*>(Application_HandleTouchEvent)},
        {"nHandleScrollEvent", "(JLjava/lang/String;FFFF)V", reinterpret_cast<void*>(Application_HandleScrollEvent)},
        {"nHandleShowPressEvent", "(JLjava/lang/String;FF)V", reinterpret_cast<void*>(Application_HandleShowPressEvent)},
        {"nHandleLongPressEvent", "(JLjava/lang/String;FF)V", reinterpret_cast<void*>(Application_HandleLongPressEvent)},
        {"nHandleSingleTapConfirmedEvent", "(JLjava/lang/String;FF)V", reinterpret_cast<void*>(Application_HandleSingleTapConfirmedEvent)},
    };

    return clazz
        ? env->RegisterNatives(clazz, methods, ABSL_ARRAYSIZE(methods))
        : JNI_ERR;
}

static int pfd[2];
static pthread_t thr;
static const char* tag = "myapp";

static void* thread_func(void*)
{
    ssize_t rdsz;
    char buf[128];
    while ((rdsz = read(pfd[0], buf, sizeof(buf) - 1)) > 0) {
        if (buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0;  /* add null-terminator */
        __android_log_write(ANDROID_LOG_INFO, tag, buf);
    }
    return 0;
}

int start_logger(const char* app_name)
{
    tag = app_name;

    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if (pthread_create(&thr, 0, thread_func, 0) == -1)
        return -1;
    pthread_detach(thr);
    return 0;
}

void create_dialog(const std::string& id)
{
    CHECK(gt_main_thread) << "expect create_dialog() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_create_dialog) {
        LOG(ERROR) << "create_dialog(): invalid JNIEnv.";
        return;
    }
    create_dialog_cb_set.insert(id);
    auto jid = jmain_jni_env->NewStringUTF(id.c_str());
    jmain_jni_env->CallStaticVoidMethod(jclass_native, jmethod_create_dialog, jid);
    jmain_jni_env->DeleteLocalRef(jid);
}
void release_dialog(const std::string& id)
{
    CHECK(gt_main_thread) << "expect release_dialog() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_release_dialog) {
        LOG(ERROR) << "release_dialog(): invalid JNIEnv.";
        return;
    }
    auto jid = jmain_jni_env->NewStringUTF(id.c_str());
    jmain_jni_env->CallStaticVoidMethod(jclass_native, jmethod_release_dialog, jid);
    jmain_jni_env->DeleteLocalRef(jid);
}

std::string string_from_jni(JNIEnv* env, jstring jstr)
{
    std::string s;
    const char* str = env->GetStringUTFChars(jstr, nullptr);
    if (str) {
        s.assign(str);
        env->ReleaseStringUTFChars(jstr, str);
    }
    return s;
}


}



