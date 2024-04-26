#include <jni.h>
#include <string>
#include <functional>
#include <android/native_window.h>

namespace android {
int kwui_jni_register_Application(JNIEnv* env);

void create_dialog(const std::string& id);
void release_dialog(const std::string& id);
}
