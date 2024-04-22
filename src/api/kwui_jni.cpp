#include "kwui_jni.h"
#include "absl/base/macros.h"
#include <android/log.h>
#include <jni.h>

JavaVM* kwui_java_vm = nullptr;

#define REGISTER_NATIVES(class_name)                     \
extern int kwui_jni_register_##class_name(JNIEnv*);    \
if (auto rc = kwui_jni_register_##class_name(env)) {   \
    __android_log_print(ANDROID_LOG_ERROR, "kwui", \
        "Failed to load natives: " #class_name);         \
    return rc;                                           \
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    kwui_java_vm = vm;
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    REGISTER_NATIVES(Application)

    return JNI_VERSION_1_6;
}
