#include "Application_jni.h"
#include "../kwui_jni.h"
#include "Application.h"
#include "ScriptEngine.h"
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

enum MessageType {
	kUndefined,
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

static int start_logger(const char* app_name);

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
    std::unique_ptr<android::WindowSurface> surface_;
    float surface_density_ = 1.0f;
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
    pthread_join(fThread, nullptr);
}

int JApplication::message_callback(int /* fd */, int /* events */, void* data) {
    auto app = (JApplication*)data;
    Message message;
    app->readMessage(&message);
    // get target surface from Message

    switch (message.fType) {
    case kSurfaceChanged: {
        app->surface_ = nullptr;
        app->surface_density_ = message.fDensity;
        sk_app::DisplayParams params;
        auto wnd_ctx = sk_app::window_context_factory::MakeGLForAndroid(message.fNativeWindow, params);
        if (wnd_ctx) {
            app->surface_.reset(new android::WindowSurface(message.fNativeWindow, std::move(wnd_ctx)));
        } else {
            LOG(ERROR) << "MakeGLForAndroid failed";
        }
        break;
    }
    case kSurfaceDestroyed: {
        app->surface_ = nullptr;
        return 0;
        break;
    }
    case kSurfaceRedrawNeeded: {
        if (app->surface_) {
            auto canvas = app->surface_->getCanvas();
            if (canvas) {
                canvas->setMatrix(SkMatrix::Scale(app->surface_density_, app->surface_density_));
                auto dlg = android::DialogAndroid::firstDialog();
                if (dlg) {
                    dlg->paint(canvas, app->surface_density_);
                }
                app->surface_->flushAndSubmit();
            }
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
    JNIEnv* env = nullptr;
    kwui_java_vm->AttachCurrentThread(&env, nullptr);

    kwui::Application app(env, me->asset_manager_);
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
        return reinterpret_cast<jlong>(g_japp);
    }
	auto asset_manager_ref = env->NewGlobalRef(asset_manager);
    const char* u8_js_path = env->GetStringUTFChars(entry_js, nullptr);
    std::string js_entry_str(u8_js_path);
    env->ReleaseStringUTFChars(entry_js, u8_js_path);

    g_japp = new JApplication(asset_manager_ref, js_entry_str);
	return reinterpret_cast<jlong>(g_japp);
}

static void Application_Release(JNIEnv* env, jobject, jlong ptr)
{
	//delete reinterpret_cast<JApplication*>(ptr);
}

static void Application_SurfaceChanged(JNIEnv* env, jobject, jlong ptr, jobject surface,
    jint /*format*/, jint /*width*/, jint /*height*/, jfloat density)
{
    LOG(INFO) << "Application_SurfaceChanged";
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceChanged);
    msg.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    msg.fDensity = density;
    me->postMessage(msg);
}

static void Application_SurfaceDestroyed(JNIEnv* env, jobject, jlong ptr)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    me->postMessage(Message(kSurfaceDestroyed));
}
static void Application_SurfaceRedrawNeeded(JNIEnv* env, jobject, jlong ptr)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    me->postMessage(Message(kSurfaceRedrawNeeded));
}
static void Application_HandleTouchEvent(JNIEnv* env, jobject, jlong ptr, jint action, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    // LOG(INFO) << "touch event: action " << action << " " << x << "," << y;
    //me->postMessage(Message(kSurfaceRedrawNeeded));
}

static void Application_HandleScrollEvent(JNIEnv* env, jobject, jlong ptr, jfloat x, jfloat y, jfloat dx, jfloat dy)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "scroll event: " << dx << "," << dy;
    Message msg(kScrollEvent);
    msg.x = x;
    msg.y = y;
    msg.dx = dx;
    msg.dy = dy;
    me->postMessage(msg);
}
static void Application_HandleShowPressEvent(JNIEnv* env, jobject, jlong ptr, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "show press event: " << x << "," << y;
    Message msg(kShowPressEvent);
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_HandleLongPressEvent(JNIEnv* env, jobject, jlong ptr, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "long press event: " << x << "," << y;
    Message msg(kLongPressEvent);
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}
static void Application_HandleSingleTapConfirmedEvent(JNIEnv* env, jobject, jlong ptr, jfloat x, jfloat y)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    LOG(INFO) << "single tap confirmed event: " << x << "," << y;
    Message msg(kSingleTapConfirmedEvent);
    msg.x = x;
    msg.y = y;
    me->postMessage(msg);
}

int kwui_jni_register_Application(JNIEnv* env) {
	static const JNINativeMethod methods[] = {
		{"nCreate" , "(Landroid/content/res/AssetManager;Ljava/lang/String;)J", reinterpret_cast<void*>(Application_Create)},
		{"nRelease", "(J)V", reinterpret_cast<void*>(Application_Release)},
        {"nSurfaceChanged", "(JLandroid/view/Surface;IIIF)V", reinterpret_cast<void*>(Application_SurfaceChanged)},
        {"nSurfaceDestroyed", "(J)V", reinterpret_cast<void*>(Application_SurfaceDestroyed)},
        {"nSurfaceRedrawNeeded", "(J)V", reinterpret_cast<void*>(Application_SurfaceRedrawNeeded)},
        {"nHandleTouchEvent", "(JIFF)V", reinterpret_cast<void*>(Application_HandleTouchEvent)},
        {"nHandleScrollEvent", "(JFFFF)V", reinterpret_cast<void*>(Application_HandleScrollEvent)},
        {"nHandleShowPressEvent", "(JFF)V", reinterpret_cast<void*>(Application_HandleShowPressEvent)},
        {"nHandleLongPressEvent", "(JFF)V", reinterpret_cast<void*>(Application_HandleLongPressEvent)},
        {"nHandleSingleTapConfirmedEvent", "(JFF)V", reinterpret_cast<void*>(Application_HandleSingleTapConfirmedEvent)},
	};

	const auto clazz = env->FindClass("com/example/myapplication/Native");
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

void kwui_request_paint()
{
    if (gt_main_thread) {
        if (g_japp && g_japp->surface_) {
            auto canvas = g_japp->surface_->getCanvas();
            if (canvas) {
                canvas->setMatrix(SkMatrix::Scale(g_japp->surface_density_, g_japp->surface_density_));
                auto dlg = android::DialogAndroid::firstDialog();
                if (dlg) {
                    dlg->paint(canvas, g_japp->surface_density_);
                }
                g_japp->surface_->flushAndSubmit();
            }
        }
    } else {
        LOG(ERROR) << "kwui_request_paint(): invalid calling thread.";
    }
}
