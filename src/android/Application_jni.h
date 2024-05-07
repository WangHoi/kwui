#include <jni.h>
#include <string>
#include <functional>
#include <stdint.h>
#include <android/native_window.h>

namespace android {
int kwui_jni_register_Application(JNIEnv* env);

void create_dialog(const std::string& id);
void release_dialog(const std::string& id);
JNIEnv* get_jni_env();
jobject get_asset_manager();
void run_in_main_thread(std::function<void()>&& func);
int application_exec();
int start_timer(int64_t interval_ms, std::function<void()> timer_func);
void stop_timer(int timer_id);

}
