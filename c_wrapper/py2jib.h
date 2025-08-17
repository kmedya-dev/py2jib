#ifndef PY2JIB_H
#define PY2JIB_H

#include <jni.h>

// Argument types that Python can pass to C++.
// This must be kept in sync with the Python ctypes definition.
enum ArgType {
    TYPE_INT = 1,
    TYPE_STRING = 2,
    // Add other types like double, bool, etc. here
};

// The structure that Python uses to pass arguments.
// This must be kept in sync with the Python ctypes definition.
struct Py2JibArg {
    int type;
    union {
        jint i_val;
        const char* s_val;
    };
};

extern "C" {

/**
 * The main entry point for Python to call static Java methods.
 * This function is exposed via the compiled shared library.
 */
void call_java_static_method(const char* class_name, const char* method_name, const char* signature, Py2JibArg* args, int arg_count);

/**
 * The JNI function that Java calls to initialize the bridge.
 */
JNIEXPORT void JNICALL
Java_com_py2jib_Py2Jib_initBridge(JNIEnv *env, jclass clazz);

}

#endif //PY2JIB_H
