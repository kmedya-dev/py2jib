#ifndef PY2JIB_H
#define PY2JIB_H

#include <jni.h>
#include <stdbool.h> // For bool type

// Argument types that Python can pass to C++ and vice-versa for return values.
// This must be kept in sync with the Python ctypes definition.
enum ArgType {
    TYPE_VOID = 0, // For void return types
    TYPE_INT = 1,
    TYPE_STRING = 2,
    TYPE_FLOAT = 3,
    TYPE_BOOLEAN = 4,
    TYPE_LONG = 5,
    TYPE_INT_ARRAY = 6,
    TYPE_STRING_ARRAY = 7,
    // Add other types like double, bool, etc. here
};

// The structure that Python uses to pass arguments.
// This must be kept in sync with the Python ctypes definition.
struct Py2JibArg {
    int type;
    union {
        jint i_val;
        const char* s_val;
        jfloat f_val;
        jboolean b_val;
        jlong l_val;
        // For arrays: pointer to data and size
        jint* int_array_val;
        const char** string_array_val;
    };
    int array_size; // Used for array types
};

// Structure for returning values from C++ to Python
// This must be kept in sync with the Python ctypes definition.
struct Py2JibReturn {
    int type;
    union {
        jint i_val;
        char* s_val; // C-string, Python will decode
        jfloat f_val;
        jboolean b_val;
        jlong l_val;
        jint* int_array_val;
        char** string_array_val;
    };
    int array_size; // Used for array types
};


extern "C" {

/**
 * The main entry point for Python to call static Java methods.
 * This function is exposed via the compiled shared library.
 * Returns a Py2JibReturn struct.
 */
struct Py2JibReturn call_java_static_method(const char* class_name, const char* method_name, const char* signature, struct Py2JibArg* args, int arg_count);

/**
 * The JNI function that Java calls to initialize the bridge.
 */
JNIEXPORT void JNICALL
Java_com_py2jib_Py2Jib_initBridge(JNIEnv *env, jclass clazz);

}

#endif //PY2JIB_H
