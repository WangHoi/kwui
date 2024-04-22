#include "Application_jni.h"
#include "../kwui_jni.h"
#include "Application.h"
#include "ScriptEngine.h"
#include "absl/base/macros.h"
#include "base/log.h"
#include <pthread.h>
#include <unistd.h>
#include <android/looper.h>
#include <android/native_window.h>
#include <string>

enum MessageType {
	kUndefined,
	kInitialize,
	kDestroy,
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
};

JApplication::JApplication(jobject asset_manager, const std::string& entry_js)
    : asset_manager_(asset_manager), entry_js_(entry_js)
{
    pipe(fPipe);
    fRunning = true;
    pthread_create(&fThread, nullptr, pthread_main, this);
}

void JApplication::postMessage(const Message& message) const {
    write(fPipe[1], &message, sizeof(message));
}

void JApplication::readMessage(Message* message) const {
    read(fPipe[0], message, sizeof(Message));
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
    case kInitialize: {
        //sk_app::DisplayParams params;
        //auto winctx = sk_app::window_context_factory::MakeGLForAndroid(message.fNativeWindow, params);
        //if (!winctx) {
        //    break;
        //}
        //*message.fWindowSurface = new WindowSurface(message.fNativeWindow, std::move(winctx));
        break;
    }
    case kDestroy: {
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
    auto me = (JApplication*)arg;
    // Looper setup
    ALooper* looper = ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);
    ALooper_addFd(looper, me->fPipe[0], 1, ALOOPER_EVENT_INPUT,
        me->message_callback, me);
    JNIEnv* env = nullptr;
    kwui_java_vm->AttachCurrentThread(&env, nullptr);

    kwui::Application app(env, me->asset_manager_);
    //LOG(INFO) << "JApplication::pthread_main(): entry_js=[" << me->entry_js_ << "]";
    if (!me->entry_js_.empty()) {
        kwui::ScriptEngine::get()->loadFile(me->entry_js_.c_str());
    }
    while (me->fRunning) {
        const int ident = ALooper_pollAll(0, nullptr, nullptr, nullptr);

        if (ident >= 0) {
            //SkDebugf("Unhandled ALooper_pollAll ident=%d !", ident);
        }
    }
    return nullptr;
}


static jlong Application_Create(JNIEnv* env, jclass, jobject asset_manager, jstring entry_js) {
	auto asset_manager_ref = env->NewGlobalRef(asset_manager);
    const char* u8_js_path = env->GetStringUTFChars(entry_js, nullptr);
    std::string js_entry_str(u8_js_path);
    env->ReleaseStringUTFChars(entry_js, u8_js_path);

	return reinterpret_cast<jlong>(new JApplication(asset_manager_ref, js_entry_str));
}

static void Application_Release(JNIEnv* env, jobject, jlong ptr) {
	delete reinterpret_cast<JApplication*>(ptr);
}

int kwui_jni_register_Application(JNIEnv* env) {
	static const JNINativeMethod methods[] = {
		{"nCreate" , "(Landroid/content/res/AssetManager;Ljava/lang/String;)J", reinterpret_cast<void*>(Application_Create)},
		{"nRelease", "(J)V", reinterpret_cast<void*>(Application_Release)},
	};

	const auto clazz = env->FindClass("com/example/myapplication/Native");
	return clazz
		? env->RegisterNatives(clazz, methods, ABSL_ARRAYSIZE(methods))
		: JNI_ERR;
}
