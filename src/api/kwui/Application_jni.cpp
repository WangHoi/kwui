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
	kRenderPicture,
};

struct Message {
	MessageType fType = kUndefined;
	ANativeWindow* fNativeWindow = nullptr;
	//SkPicture* fPicture = nullptr;
	//WindowSurface** fWindowSurface = nullptr;

	Message() {}
	Message(MessageType t) : fType(t) {}
};

static int start_logger(const char* app_name);

class JApplication {
public:
	JApplication(jobject asset_manager, const std::string& entry_js);

	void postMessage(const Message& message) const;
	void readMessage(Message* message) const;
	void release();
private:
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
};

JApplication::JApplication(jobject asset_manager, const std::string& entry_js)
    : asset_manager_(asset_manager), entry_js_(entry_js)
{
    pipe(fPipe);
    LOG(INFO) << "JApplication::JApplication() " << fPipe[0] << "/" << fPipe[1];
    fRunning = true;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

void JApplication::postMessage(const Message& message) const {
    ssize_t ret = write(fPipe[1], &message, sizeof(message));
    LOG(INFO) << "write pipe return " << ret;
}

void JApplication::readMessage(Message* message) const {
    ssize_t ret = read(fPipe[0], message, sizeof(Message));
    LOG(INFO) << "read pipe return " << ret;
}

void JApplication::release() {
    pthread_join(fThread, nullptr);
}

int JApplication::message_callback(int /* fd */, int /* events */, void* data) {
    LOG(INFO) << "message_callback";
    auto app = (JApplication*)data;
    Message message;
    app->readMessage(&message);
    // get target surface from Message

    switch (message.fType) {
    case kSurfaceChanged: {
        LOG(INFO) << "Surface changed";
        app->surface_ = nullptr;
        sk_app::DisplayParams params;
        auto wnd_ctx = sk_app::window_context_factory::MakeGLForAndroid(message.fNativeWindow, params);
        if (wnd_ctx) {
            LOG(INFO) << "creating WindowSurface";
            app->surface_.reset(new android::WindowSurface(message.fNativeWindow, std::move(wnd_ctx)));
        } else {
            LOG(ERROR) << "MakeGLForAndroid failed";
        }
        break;
    }
    case kSurfaceDestroyed: {
        LOG(INFO) << "Surface destroyed";
        app->surface_ = nullptr;
        //SkDebugf("surface destroyed, shutting down thread");
        //JApplication->fRunning = false;
        //if (auto* windowSurface = reinterpret_cast<Surface*>(*message.fWindowSurface)) {
        //    windowSurface->release(nullptr);
        //    delete windowSurface;
        //}
        return 0;
        break;
    }
    case kRenderPicture: {
        //sk_sp<SkPicture> picture(message.fPicture);
        //if (auto* windowSurface = reinterpret_cast<Surface*>(*message.fWindowSurface)) {
        //    windowSurface->getCanvas()->drawPicture(picture);
        //    windowSurface->flushAndSubmit();
        //}
        break;
    }
    default: {
        // do nothing
    }
    }

    return 1;  // continue receiving callbacks
}

void* JApplication::pthread_main(void* arg) {
    start_logger("script");
    auto me = (JApplication*)arg;
    // Looper setup
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, me->fPipe[0], ALOOPER_POLL_CALLBACK, ALOOPER_EVENT_INPUT,
        me->message_callback, me);
    JNIEnv* env = nullptr;
    kwui_java_vm->AttachCurrentThread(&env, nullptr);

    kwui::Application app(env, me->asset_manager_);
    //LOG(INFO) << "JApplication::pthread_main(): entry_js=[" << me->entry_js_ << "]";
    if (!me->entry_js_.empty()) {
        kwui::ScriptEngine::get()->loadFile(me->entry_js_.c_str());
    }
    while (me->fRunning) {
        const int ident = ALooper_pollOnce(2000, nullptr, nullptr, nullptr);

        //if (ident != ALOOPER_POLL_TIMEOUT) {
            //SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
            LOG(INFO) << "ALooper_pollOnce return " << ident;
        //}
        if (me->surface_) {
            auto canvas = me->surface_->getCanvas();
            if (canvas) {
                //LOG(INFO) << "clear canvas";
                canvas->clear(SK_ColorBLUE);
                me->surface_->flushAndSubmit();
            }
        }
    }
    return nullptr;
}

static jlong Application_Create(JNIEnv* env, jclass, jobject asset_manager, jstring entry_js)
{
	auto asset_manager_ref = env->NewGlobalRef(asset_manager);
    const char* u8_js_path = env->GetStringUTFChars(entry_js, nullptr);
    std::string js_entry_str(u8_js_path);
    env->ReleaseStringUTFChars(entry_js, u8_js_path);

	return reinterpret_cast<jlong>(new JApplication(asset_manager_ref, js_entry_str));
}

static void Application_Release(JNIEnv* env, jobject, jlong ptr)
{
	delete reinterpret_cast<JApplication*>(ptr);
}

static void Application_SurfaceChanged(JNIEnv* env, jobject, jlong ptr, jobject surface,
    jint /*format*/, jint /*width*/, jint /*height*/)
{
    LOG(INFO) << "Application_SurfaceChanged";
    auto me = reinterpret_cast<JApplication*>(ptr);
    Message msg(kSurfaceChanged);
    msg.fNativeWindow = ANativeWindow_fromSurface(env, surface);
    me->postMessage(msg);
}

static void Application_SurfaceDestroyed(JNIEnv* env, jobject, jlong ptr)
{
    auto me = reinterpret_cast<JApplication*>(ptr);
    me->postMessage(Message(kSurfaceDestroyed));
}

int kwui_jni_register_Application(JNIEnv* env) {
	static const JNINativeMethod methods[] = {
		{"nCreate" , "(Landroid/content/res/AssetManager;Ljava/lang/String;)J", reinterpret_cast<void*>(Application_Create)},
		{"nRelease", "(J)V", reinterpret_cast<void*>(Application_Release)},
        {"nSurfaceChanged", "(JLandroid/view/Surface;III)V", reinterpret_cast<void*>(Application_SurfaceChanged)},
        {"nSurfaceDestroyed", "(J)V", reinterpret_cast<void*>(Application_SurfaceDestroyed)},
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
