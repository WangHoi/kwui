#include "Application_jni.h"
#include "kwui_jni.h"
#include "api/kwui/Application.h"
#include "api/kwui/ScriptEngine.h"
#include "absl/base/macros.h"
#include "absl/time/time.h"
#include "base/log.h"
#include "tools/sk_app/DisplayParams.h"
#include "tools/sk_app/WindowContext.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "tools/sk_app/android/WindowContextFactory_android.h"
#include "android/SurfaceAndroid.h"
#include "android/DialogAndroid.h"
#include "absl/base/internal/per_thread_tls.h"
#include "absl/time/clock.h"
#include <pthread.h>
#include <unistd.h>
#include <android/log.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <android/keycodes.h>
#include <dlfcn.h>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace android {

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
    kRunInMainThread,
    kCommitTextEvent,
    kKeyDownEvent,
    kKeyUpEvent,
};

struct Message {
    MessageType fType = kUndefined;
    std::string* id = nullptr;
    ANativeWindow* fNativeWindow = nullptr;
    float fDensity = 1.0f;
    float x, y;
    float dx, dy;
    std::function<void()>* func = nullptr;
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
static jmethodID jmethod_get_main_shared_object = nullptr;
static jmethodID jmethod_get_main_function = nullptr;
static jmethodID jmethod_show_text_input = nullptr;
static jmethodID jmethod_hide_text_input = nullptr;

static int start_logger(const char* app_name);
static std::string string_from_jni(JNIEnv* env, jstring jstr);
static std::string get_main_shared_object();
static std::string get_main_function();

static scene2d::VKey make_vkey(int vk);

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
        auto dlg = DialogAndroid::findDialogById(*message.id);
        if (dlg) {
            dlg->handleSurfaceChanged(message.fNativeWindow, message.fDensity);
        } else {
            LOG(ERROR) << "kSurfaceChanged: dialog " << *message.id << " not found.";
        }
        delete message.id;
        break;
    }
    case kSurfaceDestroyed: {
        auto dlg = DialogAndroid::findDialogById(*message.id);
        if (dlg) {
            dlg->handleSurfaceDestroyed();
        } else {
            LOG(ERROR) << "kSurfaceDestroyed: dialog " << *message.id << " not found.";
        }
        delete message.id;
        break;
    }
    case kSurfaceRedrawNeeded: {
        auto dlg = DialogAndroid::findDialogById(*message.id);
        if (dlg) {
            dlg->handleSurfaceRedrawNeeded();
        } else {
            LOG(ERROR) << "kSurfaceDestroyed: dialog " << *message.id << " not found.";
        }
        delete message.id;
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
    case kRunInMainThread: {
        if (message.func) {
            (*message.func)();
            delete message.func;
        }
        break;
    }
    case kCommitTextEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleImeCommit(base::EncodingManager::UTF8ToWide(*message.id));
        }
        delete message.id;
        break;
    }
    case kKeyDownEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleKeyDown(make_vkey((int)message.x));
        }
        delete message.id;
        break;
    }
    case kKeyUpEvent: {
        auto dlg = android::DialogAndroid::firstDialog();
        if (dlg) {
            dlg->handleKeyUp(make_vkey((int)message.x));
        }
        delete message.id;
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

    //kwui::Application app(jmain_jni_env, me->asset_manager_);
    //LOG(INFO) << "JApplication::pthread_main(): entry_js=[" << me->entry_js_ << "]";
    // if (!me->entry_js_.empty()) {
    //     kwui::ScriptEngine::get()->loadFile(me->entry_js_.c_str());
    // }
    int (*kwui_main)(int, char**) = nullptr;
    void* dl_handle = dlopen(get_main_shared_object().c_str(), RTLD_GLOBAL);
    if (dl_handle) {
        kwui_main = (int (*)(int, char**))dlsym(dl_handle, get_main_function().c_str()); 
    }
    if (kwui_main) {
        kwui_main(0, nullptr);
    } else {
        LOG(ERROR) << "native function [" << get_main_function() << "] not found in shared object [" << get_main_shared_object() << "]";
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
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
    msg.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    msg.fDensity = density;
    me->postMessage(msg);
}

static void Application_SurfaceDestroyed(JNIEnv* env, jobject, jlong ptr, jstring jid)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceDestroyed);
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
    me->postMessage(msg);
}
static void Application_SurfaceRedrawNeeded(JNIEnv* env, jobject, jlong ptr, jstring jid)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceRedrawNeeded);
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
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
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
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
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_HandleLongPressEvent(JNIEnv* env, jobject, jlong ptr, jstring jid, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "long press event: " << x << "," << y;
    Message msg(kLongPressEvent);
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_HandleSingleTapConfirmedEvent(JNIEnv* env, jobject, jlong ptr, jstring jid, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "single tap confirmed event: " << x << "," << y;
    Message msg(kSingleTapConfirmedEvent);
    msg.id = new std::string(std::move(string_from_jni(env, jid)));
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_KeyboardFocusLost(JNIEnv* env, jobject)
{
    LOG(INFO) << "KeyboardFocusLost() ";
}
static void Application_HandleKeyDown(JNIEnv* env, jobject, jlong ptr, int key_code)
{
    LOG(INFO) << "HandleKeyDown() " << key_code;
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kKeyDownEvent);
    // msg.id = new std::string(std::move(string_from_jni(env, jid)));
    msg.x = key_code;
    me->postMessage(msg);
}
static void Application_HandleKeyUp(JNIEnv* env, jobject, jlong ptr, int key_code)
{
    LOG(INFO) << "HandleKeyUp() " << key_code;
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kKeyUpEvent);
    // msg.id = new std::string(std::move(string_from_jni(env, jid)));
    msg.x = key_code;
    me->postMessage(msg);
}
static jboolean Application_SoftReturnKey(JNIEnv* env, jobject)
{
    LOG(INFO) << "SoftReturnKey() ";
    return JNI_TRUE;
}
static void Application_CommitText(JNIEnv* env, jobject, jlong ptr, jstring jtext, jint cursor_pos)
{
    std::string text = string_from_jni(env, jtext);
    LOG(INFO) << "CommitText() \"" << text << "\" cursor " << cursor_pos;
    // TODO(android): add |Message.text|
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kCommitTextEvent);
    msg.id = new std::string(std::move(text));
    me->postMessage(msg);
}
static void Application_GenerateScancodeForUnichar(JNIEnv* env, jobject, jchar unichar)
{
    LOG(INFO) << "GenerateScancodeForUnichar() " << unichar;
}
int kwui_jni_register_Application(JNIEnv* env)
{
    const auto clazz = env->FindClass("prj/kwui/Native");
    if (clazz) {
        jclass_native = reinterpret_cast<jclass>(env->NewGlobalRef(reinterpret_cast<jobject>(clazz)));
        jmethod_create_dialog = env->GetStaticMethodID(jclass_native, "jCreateDialog", "(Ljava/lang/String;)V");
        jmethod_release_dialog = env->GetStaticMethodID(jclass_native, "jReleaseDialog", "(Ljava/lang/String;)V");
        jmethod_get_main_shared_object = env->GetStaticMethodID(jclass_native, "jGetMainSharedObject", "()Ljava/lang/String;");
        jmethod_get_main_function = env->GetStaticMethodID(jclass_native, "jGetMainFunction", "()Ljava/lang/String;");
        jmethod_show_text_input = env->GetStaticMethodID(jclass_native, "jShowTextInput", "(IIII)V");
        jmethod_hide_text_input = env->GetStaticMethodID(jclass_native, "jHideTextInput", "()V");
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
        {"nKeyboardFocusLost", "()V", reinterpret_cast<void*>(Application_KeyboardFocusLost)},
        {"nHandleKeyDown", "(JI)V", reinterpret_cast<void*>(Application_HandleKeyDown)},
        {"nHandleKeyUp", "(JI)V", reinterpret_cast<void*>(Application_HandleKeyUp)},
        {"nSoftReturnKey", "()Z", reinterpret_cast<void*>(Application_SoftReturnKey)},
        {"nCommitText", "(JLjava/lang/String;I)V", reinterpret_cast<void*>(Application_CommitText)},
        {"nGenerateScancodeForUnichar", "(C)V", reinterpret_cast<void*>(Application_GenerateScancodeForUnichar)},
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

std::string get_main_shared_object()
{
    CHECK(gt_main_thread) << "expect get_main_shared_object() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_get_main_shared_object) {
        LOG(ERROR) << "get_main_shared_object(): invalid JNIEnv.";
        return "";
    }
    auto jstr = jmain_jni_env->CallStaticObjectMethod(jclass_native, jmethod_get_main_shared_object);
    auto str = string_from_jni(jmain_jni_env, (jstring)jstr);
    jmain_jni_env->DeleteLocalRef(jstr);
    return str;
}
std::string get_main_function()
{
    CHECK(gt_main_thread) << "expect get_main_function() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_get_main_function) {
        LOG(ERROR) << "get_main_function(): invalid JNIEnv.";
        return "";
    }
    auto jstr = jmain_jni_env->CallStaticObjectMethod(jclass_native, jmethod_get_main_function);
    auto str = string_from_jni(jmain_jni_env, (jstring)jstr);
    jmain_jni_env->DeleteLocalRef(jstr);
    return str;
}

void create_dialog(const std::string& id)
{
    CHECK(gt_main_thread) << "expect create_dialog() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_create_dialog) {
        LOG(ERROR) << "create_dialog(): invalid JNIEnv.";
        return;
    }
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
    size_t len = env->GetStringUTFLength(jstr);
    const char* str = env->GetStringUTFChars(jstr, nullptr);
    if (str) {
        s.assign(str, str + len);
        env->ReleaseStringUTFChars(jstr, str);
    }
    return s;
}

JNIEnv* get_jni_env()
{
    return jmain_jni_env;
}
jobject get_asset_manager()
{
    return g_japp ? g_japp->asset_manager_ : nullptr;
}
void run_in_main_thread(std::function<void()>&& func)
{
    if (gt_main_thread) {
        func();
    } else if (g_japp) {
        Message msg(kRunInMainThread);
        msg.func = new std::function<void()>(std::move(func));
        g_japp->postMessage(msg);
    } else {
        LOG(ERROR) << "run_in_main_thread(): invalid JApplication.";
    }
}
static void run_timer_funcs();
int application_exec()
{
    while (g_japp && g_japp->fRunning) {
        const int ident = ALooper_pollOnce(0, nullptr, nullptr, nullptr);

        //if (ident != ALOOPER_POLL_TIMEOUT) {
            //SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        //LOG(INFO) << "ALooper_pollOnce return " << ident;
        if (ident == 1) {
            g_japp->message_callback(0, 0, g_japp);
        }
        run_timer_funcs();
        //}
    }
    return 0;
}

struct TimerContext {
    int64_t interval_ms;
    absl::Time next_instant;
    std::function<void()> func;
};
static int g_next_timer_id = 1;
static std::unordered_map<int, TimerContext> g_timer_map;
static std::vector<int> g_timeout_timer_ids;

int start_timer(int64_t interval_ms, std::function<void()> timer_func)
{
    if (!gt_main_thread) {
        LOG(ERROR) << "start_timer() in non-main thread not supported.";
        return 0;
    }
    TimerContext ctx;
    ctx.interval_ms = interval_ms;
    ctx.next_instant = absl::Now();
    ctx.func = timer_func;
    int id = g_next_timer_id++;
    g_timer_map[id] = ctx;
    return id;
}
void stop_timer(int timer_id)
{
    if (!gt_main_thread) {
        LOG(ERROR) << "stop_timer() in non-main thread not supported.";
        return;
    }
    g_timer_map.erase(timer_id);
}
void run_timer_funcs()
{
    if (g_timer_map.empty())
        return;
    auto now = absl::Now();
    for (auto& p : g_timer_map) {
        if (now >= p.second.next_instant) {
            p.second.next_instant += absl::Milliseconds(p.second.interval_ms);
            g_timeout_timer_ids.push_back(p.first);
        }
    }
    for (auto id : g_timeout_timer_ids) {
        auto it = g_timer_map.find(id);
        if (it != g_timer_map.end()) {
            it->second.func();
        }
    }
    g_timeout_timer_ids.clear();
}
void show_text_input(float x, float y, float w, float h)
{
    CHECK(gt_main_thread) << "expect show_text_input() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_show_text_input) {
        LOG(ERROR) << "show_text_input(): invalid JNIEnv.";
        return;
    }
    LOG(INFO) << "start_text_input() " << scene2d::RectF::fromXYWH(x, y, w, h);
    jmain_jni_env->CallStaticVoidMethod(jclass_native,
        jmethod_show_text_input,
        (jint)x, (jint)y, (jint)w, (jint)h);
}
void hide_text_input()
{
    CHECK(gt_main_thread) << "expect hide_text_input() from main thread";
    if (!jmain_jni_env || !jclass_native || !jmethod_hide_text_input) {
        LOG(ERROR) << "hide_text_input(): invalid JNIEnv.";
        return;
    }
    LOG(INFO) << "hide_text_input() ";
    jmain_jni_env->CallStaticVoidMethod(jclass_native, jmethod_hide_text_input);
}


scene2d::VKey make_vkey(int vk) {
    static const struct {
        int      fVK;
        scene2d::VKey fKey;
    } gPair[] = {
        { AKEYCODE_ENTER,  		scene2d::VKey::Return   },
        { AKEYCODE_ESCAPE,  	scene2d::VKey::Escape   },
        { AKEYCODE_DEL,			scene2d::VKey::Back     },
        // { VK_DELETE,  scene2d::VKey::Delete   },
        { AKEYCODE_DPAD_UP,     scene2d::VKey::Up       },
        { AKEYCODE_DPAD_DOWN,   scene2d::VKey::Down     },
        { AKEYCODE_DPAD_LEFT,   scene2d::VKey::Left     },
        { AKEYCODE_DPAD_RIGHT,	scene2d::VKey::Right    },
        { AKEYCODE_TAB,     	scene2d::VKey::Tab      },
        { AKEYCODE_PAGE_UP,   	scene2d::VKey::PageUp   },
        { AKEYCODE_PAGE_DOWN,   scene2d::VKey::PageDown },
        { AKEYCODE_MOVE_HOME,   scene2d::VKey::Home     },
        { AKEYCODE_MOVE_END,    scene2d::VKey::End      },
        { AKEYCODE_SPACE,   	scene2d::VKey::Space    },
        // { VK_SHIFT,   scene2d::VKey::Shift    },
        // { VK_CONTROL, scene2d::VKey::Ctrl     },
        // { VK_MENU,    scene2d::VKey::Option   },
        { AKEYCODE_A,        	scene2d::VKey::A        },
        { AKEYCODE_B,        	scene2d::VKey::B        },
        { AKEYCODE_C,        	scene2d::VKey::C        },
        { AKEYCODE_V,        	scene2d::VKey::V        },
        { AKEYCODE_X,        	scene2d::VKey::X        },
        { AKEYCODE_Y,        	scene2d::VKey::Y        },
        { AKEYCODE_Z,        	scene2d::VKey::Z        },
    };
    for (size_t i = 0; i < ABSL_ARRAYSIZE(gPair); i++) {
        if (gPair[i].fVK == vk) {
            return gPair[i].fKey;
        }
    }
    return scene2d::VKey::Invalid;
}

}
