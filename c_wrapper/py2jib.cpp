#include "py2jib.h"
#include <jni.h>
#include <string>
#include <vector>
#include <android/log.h>
#include <cstring> // For memcpy

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
    // Attach current thread to JVM if not already attached
    jint getEnvStat = g_vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        LOGI("Attaching current thread to JVM");
        if (g_vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
            LOGE("Failed to attach current thread to JVM");
            return nullptr;
        }
    } else if (getEnvStat == JNI_EVERSION) {
        LOGE("JNI version not supported");
        return nullptr;
    }
    return env;
}

// Called from Java to confirm the bridge is initialized.
JNIEXPORT void JNICALL
Java_com_py2jib_Py2Jib_initBridge(JNIEnv *env, jclass clazz) {
    LOGI("Py2Jib C++ bridge initialized.");
}

// Helper to convert jstring to C-string (caller must free)
char* jstring_to_cstring(JNIEnv *env, jstring jstr) {
    if (!jstr) return nullptr;
    const char *cstr = env->GetStringUTFChars(jstr, nullptr);
    char* result = strdup(cstr);
    env->ReleaseStringUTFChars(jstr, cstr);
    return result;
}

// Helper to convert C-string to jstring
jstring cstring_to_jstring(JNIEnv *env, const char* cstr) {
    if (!cstr) return nullptr;
    return env->NewStringUTF(cstr);
}

// Memory management functions for Python
extern "C" {
void free_string(char* s) {
    free(s);
}

void free_int_array(jint* arr) {
    delete[] arr;
}

void free_string_array_ptr(char** arr) {
    if (arr) {
        // Individual strings are freed by Python, this frees the array of pointers
        delete[] arr;
    }
}
}

// The main entry point for Python calls.
struct Py2JibReturn call_java_static_method(const char* class_name_cstr, const char* method_name_cstr, const char* signature_cstr, struct Py2JibArg* args, int arg_count) {
    JNIEnv* env = get_jni_env();
    struct Py2JibReturn result_py = {TYPE_VOID};

    if (env == nullptr) return result_py;

    jclass target_class = env->FindClass(class_name_cstr);
    if (target_class == nullptr) {
        LOGE("Class not found: %s", class_name_cstr);
        return result_py;
    }

    jmethodID method_id = env->GetStaticMethodID(target_class, method_name_cstr, signature_cstr);
    if (method_id == nullptr) {
        LOGE("Static method '%s' with signature '%s' not found in class %s", method_name_cstr, signature_cstr, class_name_cstr);
        if (env->ExceptionCheck()) env->ExceptionClear();
        return result_py;
    }

    std::vector<jvalue> jni_args(arg_count);
    std::vector<jobject> local_refs_to_delete; // To manage local references created for args

    for (int i = 0; i < arg_count; ++i) {
        switch (args[i].type) {
            case TYPE_INT:
                jni_args[i].i = args[i].i_val;
                break;
            case TYPE_STRING:
                jni_args[i].l = cstring_to_jstring(env, args[i].s_val);
                if (jni_args[i].l) local_refs_to_delete.push_back(jni_args[i].l);
                break;
            case TYPE_FLOAT:
                jni_args[i].f = args[i].f_val;
                break;
            case TYPE_BOOLEAN:
                jni_args[i].z = args[i].b_val;
                break;
            case TYPE_LONG:
                jni_args[i].j = args[i].l_val;
                break;
            case TYPE_INT_ARRAY: {
                jintArray j_array = env->NewIntArray(args[i].array_size);
                if (j_array) {
                    env->SetIntArrayRegion(j_array, 0, args[i].array_size, args[i].int_array_val);
                    jni_args[i].l = j_array;
                    local_refs_to_delete.push_back(j_array);
                }
                break;
            }
            case TYPE_STRING_ARRAY: {
                jclass stringClass = env->FindClass("java/lang/String");
                jobjectArray j_array = env->NewObjectArray(args[i].array_size, stringClass, nullptr);
                if (j_array) {
                    for (int k = 0; k < args[i].array_size; ++k) {
                        jstring j_str = cstring_to_jstring(env, args[i].string_array_val[k]);
                        env->SetObjectArrayElement(j_array, k, j_str);
                        if (j_str) env->DeleteLocalRef(j_str); // Delete temporary string ref
                    }
                    jni_args[i].l = j_array;
                    local_refs_to_delete.push_back(j_array);
                }
                if (stringClass) env->DeleteLocalRef(stringClass);
                break;
            }
            default:
                LOGE("Unsupported argument type: %d", args[i].type);
                // Clean up created objects before returning
                for (jobject ref : local_refs_to_delete) {
                    env->DeleteLocalRef(ref);
                }
                return result_py;
        }
    }

    // Determine return type from signature
    char return_type_char = signature_cstr[strlen(signature_cstr) - 1];
    if (signature_cstr[strlen(signature_cstr) - 2] == ']') { // Check for array return type
        // This is a simplified check, a full parser would be more robust
        if (signature_cstr[strlen(signature_cstr) - 3] == 'I') {
            return_type_char = '['; // int array
        } else if (signature_cstr[strlen(signature_cstr) - 3] == ';' && signature_cstr[strlen(signature_cstr) - 1] == ';') {
            return_type_char = 'L'; // object array (e.g. String[])
        }
    }

    switch (return_type_char) {
        case 'V': // void
            env->CallStaticVoidMethodA(target_class, method_id, jni_args.data());
            result_py.type = TYPE_VOID;
            break;
        case 'I': // int
            result_py.i_val = env->CallStaticIntMethodA(target_class, method_id, jni_args.data());
            result_py.type = TYPE_INT;
            break;
        case 'F': // float
            result_py.f_val = env->CallStaticFloatMethodA(target_class, method_id, jni_args.data());
            result_py.type = TYPE_FLOAT;
            break;
        case 'Z': // boolean
            result_py.b_val = env->CallStaticBooleanMethodA(target_class, method_id, jni_args.data());
            result_py.type = TYPE_BOOLEAN;
            break;
        case 'J': // long
            result_py.l_val = env->CallStaticLongMethodA(target_class, method_id, jni_args.data());
            result_py.type = TYPE_LONG;
            break;
        case ';': // Object (e.g., String, or other Java objects)
        case 'L': {
            jobject returned_obj = env->CallStaticObjectMethodA(target_class, method_id, jni_args.data());
            if (returned_obj) {
                // Check if it's a String
                if (env->IsInstanceOf(returned_obj, env->FindClass("java/lang/String"))) {
                    result_py.s_val = jstring_to_cstring(env, (jstring)returned_obj);
                    result_py.type = TYPE_STRING;
                } else if (env->IsInstanceOf(returned_obj, env->FindClass("[I"))) { // int[]
                    jintArray j_int_array = (jintArray)returned_obj;
                    result_py.array_size = env->GetArrayLength(j_int_array);
                    jint* elements = env->GetIntArrayElements(j_int_array, nullptr);
                    result_py.int_array_val = new jint[result_py.array_size];
                    memcpy(result_py.int_array_val, elements, result_py.array_size * sizeof(jint));
                    env->ReleaseIntArrayElements(j_int_array, elements, JNI_ABORT);
                    result_py.type = TYPE_INT_ARRAY;
                } else if (env->IsInstanceOf(returned_obj, env->FindClass("[Ljava/lang/String;"))) { // String[]
                    jobjectArray j_string_array = (jobjectArray)returned_obj;
                    result_py.array_size = env->GetArrayLength(j_string_array);
                    result_py.string_array_val = new char*[result_py.array_size];
                    for (int i = 0; i < result_py.array_size; ++i) {
                        jstring j_str_elem = (jstring)env->GetObjectArrayElement(j_string_array, i);
                        result_py.string_array_val[i] = jstring_to_cstring(env, j_str_elem);
                        if (j_str_elem) env->DeleteLocalRef(j_str_elem);
                    }
                    result_py.type = TYPE_STRING_ARRAY;
                } else {
                    LOGE("Unsupported object return type. Returning null.");
                    // For now, just return void for unsupported objects
                    result_py.type = TYPE_VOID;
                }
                env->DeleteLocalRef(returned_obj);
            }
            break;
        }
        default:
            LOGE("Unsupported return type in signature: %c", return_type_char);
            result_py.type = TYPE_VOID;
            break;
    }

    // Clean up local references created for arguments
    for (jobject ref : local_refs_to_delete) {
        env->DeleteLocalRef(ref);
    }

    if (env->ExceptionCheck()) {
        LOGE("Exception occurred while calling %s.%s", class_name_cstr, method_name_cstr);
        env->ExceptionDescribe();
        env->ExceptionClear();
        // Consider how to propagate this to Python
    }
    return result_py;
}
