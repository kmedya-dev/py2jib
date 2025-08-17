#include "py2jib.h"
#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>

#define LOG_TAG "Py2Jib"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static JavaVM* g_vm = nullptr;

// JNI_OnLoad is called when the library is loaded.
// We use it to cache the JavaVM instance.
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_vm = vm;
    return JNI_VERSION_1_6;
}

// Helper function to get the JNIEnv for the current thread.
JNIEnv* get_jni_env() {
    JNIEnv* env;
    if (g_vm == nullptr) {
        LOGE("JavaVM is null. The library was not loaded correctly.");
        return nullptr;
    }
    if (g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        LOGE("Failed to get JNIEnv for the current thread.");
        return nullptr;
    }
    return env;
}

// Called from Java to confirm the bridge is initialized.
JNIEXPORT void JNICALL
Java_com_py2jib_Py2Jib_initBridge(JNIEnv *env, jclass clazz) {
    LOGI("Py2Jib C++ bridge initialized.");
}

// The main entry point for Python calls.
void call_java_static_method(const char* class_name_cstr, const char* method_name_cstr, const char* signature_cstr, Py2JibArg* args, int arg_count) {
    JNIEnv* env = get_jni_env();
    if (env == nullptr) return;

    jclass target_class = env->FindClass(class_name_cstr);
    if (target_class == nullptr) {
        LOGE("Class not found: %s", class_name_cstr);
        return;
    }

    jmethodID method_id = env->GetStaticMethodID(target_class, method_name_cstr, signature_cstr);
    if (method_id == nullptr) {
        LOGE("Static method '%s' with signature '%s' not found in class %s", method_name_cstr, signature_cstr, class_name_cstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        return;
    }

    std::vector<jvalue> jni_args(arg_count);
    for (int i = 0; i < arg_count; ++i) {
        switch (args[i].type) {
            case TYPE_INT:
                jni_args[i].i = args[i].i_val;
                break;
            case TYPE_STRING:
                jni_args[i].l = env->NewStringUTF(args[i].s_val);
                break;
            default:
                LOGE("Unsupported argument type: %d", args[i].type);
                // Clean up created objects before returning
                for (int j = 0; j < i; ++j) {
                    if (args[j].type == TYPE_STRING) env->DeleteLocalRef((jobject)jni_args[j].l);
                }
                return;
        }
    }

    // For now, we only handle void return types, as in the Toast example.
    // A full implementation would inspect the signature's return type.
    env->CallStaticVoidMethodA(target_class, method_id, jni_args.data());

    // Clean up local references (the strings we created)
    for (int i = 0; i < arg_count; ++i) {
        if (args[i].type == TYPE_STRING) {
            env->DeleteLocalRef((jobject)jni_args[i].l);
        }
    }

    if (env->ExceptionCheck()) {
        LOGE("Exception occurred while calling %s.%s", class_name_cstr, method_name_cstr);
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
