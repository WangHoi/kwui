#include <jni.h>
#include <string>
#include <functional>
#include <android/native_window.h>

namespace android {
int kwui_jni_register_Application(JNIEnv* env);
void kwui_request_paint();

using CreateDialogCb = std::function<void(const std::string&, ANativeWindow*)>;
void create_dialog(const std::string& id, CreateDialogCb create_cb);
void release_dialog(const std::string& id);
}
