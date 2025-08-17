# Py2Jib: Python to Java Interface Bridge

Py2Jib is a custom Python-to-Java bridge designed for Android. It allows you to call Java code from Python with a simple, high-level API.

## Features

- **Easy Calls:** `jni.ClassName.method(args)`
- **WebView Integration:** Run JavaScript in a WebView from Python.
- **Android API Access:** Helpers for Toast, Sensors, Camera, Clipboard.
- **Automatic Type Conversion:** Handles `int`, `float`, `string`, and common Java objects.
- **Java Exception Propagation:** Java exceptions are raised as Python exceptions.

## Project Structure

```
.
├── README.md
├── build
│   └── CMakeLists.txt
├── c_wrapper
│   ├── py2jib.cpp
│   └── py2jib.h
├── examples
│   ├── sensor_example.py
│   ├── toast_example.py
│   └── webview_example.py
├── java
│   └── com
│       └── py2jib
│           ├── Py2Jib.java
│           ├── SensorHelper.java
│           ├── ToastHelper.java
│           └── WebViewHelper.java
├── python
│   └── py2jib
│       ├── __init__.py
│       └── android.py
└── .github
    └── workflows
        └── build.yml
```

## Setup and Building

1.  **Prerequisites:**
    *   Android NDK
    *   CMake
    *   Python 3

2.  **Build the C++ Wrapper:**
    *   Navigate to the `build` directory.
    *   Run CMake and make to compile the JNI library (`libpy2jib.so`).

    ```bash
    cd build
    cmake .. -DANDROID_ABI=arm64-v8a -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake -DANDROID_PLATFORM=android-21
    make
    ```

3.  **Include in your Android Project:**
    *   Place the compiled `libpy2jib.so` in your Android project's `jniLibs` directory.
    *   Add the `java` and `python` source directories to your project.

## Usage

```python
from py2jib import jni
from py2jib.android import Toast

# Show a toast
Toast.show("Hello from Python!", Toast.LENGTH_LONG)

# Call a static Java method
System = jni.java.lang.System
currentTime = System.currentTimeMillis()
print(f"Current time from Java: {currentTime}")
```
