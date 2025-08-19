# Py2Jib: Python to Java Interface Bridge

Py2Jib is a custom Python-to-Java bridge designed for Android. It allows you to call Java code from Python with a simple, high-level API.

## Features

- **Easy Calls:** Simple `jni.ClassName.method(args)` syntax.
- **Android API Access:** Comes with helper examples for `Toast`, `WebView`, and `Sensors`.
- **Extensible:** The bridge can be used to call any static Java method.
- **Automatic Signature Generation:** Automatically creates JNI method signatures for `int` and `str` types.
- **CI/CD:** Includes a GitHub Actions workflow to build and test the C++ wrapper.

## Project Structure

```
.
├── CMakeLists.txt
├── LICENSE
├── README.md
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

## Setup and Building (Manual)

These instructions are for building the C++ shared library (`libpy2jib.so`) manually. For integration into a full Android app, see the section on `python-for-android`.

1.  **Prerequisites:**
    *   Android NDK (e.g., r27d)
    *   CMake

2.  **Configure and Build:**
    *   From the project root, create a build directory.
    *   Run CMake to configure the project, pointing to the NDK toolchain file.
    *   Run make (or another build generator like Ninja) to compile.

    ```bash
    # Create an empty build directory
    mkdir -p build && cd build

    # Configure CMake. Replace $ANDROID_NDK with the path to your NDK.
    cmake .. -DANDROID_ABI=arm64-v8a \
             -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
             -DANDROID_PLATFORM=android-21

    # Compile the library
    cmake --build .
    ```
    The compiled `libpy2jib.so` will be in the `build` directory.

## Usage

The following example shows how to initialize the bridge and call a static Java method.

```python
import py2jib
from py2jib import jni
from py2jib.android import Toast

# 1. Initialize the bridge by providing the path to the compiled library.
#    This only needs to be done once when your app starts.
py2jib.init("/path/to/your/app/libs/libpy2jib.so")

# 2. In your Android app\\'s Java code, initialize the bridge context.
#    This should be done on startup, for example in your Application.onCreate().
#    com.py2jib.Py2Jib.init(this);

# 3. Now you can call Java methods from Python.
Toast.show("Hello from Python!", Toast.LENGTH_LONG)

# Example of calling a standard Java library
System = jni.java.lang.System
# Note: Calling methods with return values is not yet implemented.
# currentTime = System.currentTimeMillis()
# print(f"Current time from Java: {currentTime}")
```


## Integration with DroidBuilder

Py2Jib is designed to be integrated into a larger Android application project built with a tool like "DroidBuilder". DroidBuilder would handle the overall Android project structure, compilation, and packaging, while Py2Jib provides the Python-to-Java bridging capabilities.

To integrate Py2Jib into a DroidBuilder project:

1.  **Include Py2Jib Java Sources:** Ensure DroidBuilder's build configuration includes the Java source files located in the `java/` directory of this project.
2.  **Include Py2Jib Native Library:** Configure DroidBuilder to compile the C++ wrapper using the `CMakeLists.txt` file in this project's root. The resulting `libpy2jib.so` should be bundled into your application's `jniLibs` directory.
3.  **Bundle Python Code:** Ensure your Python application code (including the `python/py2jib` module) is bundled into the Android application's assets or resources, where the Python interpreter can find it at runtime.
4.  **Initialize Bridge:** As shown in the Usage example, you'll need to initialize the Py2Jib bridge from both your Android app's Java code and your Python code at startup.

```
