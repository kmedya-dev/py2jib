import ctypes
import os

# --- CTypes Definitions ---

# This enum must be kept in sync with c_wrapper/py2jib.h
class ArgType:
    TYPE_INT = 1
    TYPE_STRING = 2

# This structure must be kept in sync with c_wrapper/py2jib.h
class Py2JibArg(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int),
                # Using a union for the value
                ("i_val", ctypes.c_int),
                ("s_val", ctypes.c_char_p)]

# --- Library Loading ---

_lib = None

def init(library_path):
    """Loads the compiled C++ shared library."""
    global _lib
    try:
        _lib = ctypes.CDLL(library_path)
    except OSError as e:
        print(f"Error loading library at {library_path}: {e}")
        print("Please ensure libpy2jib.so is compiled and the path is correct.")
        return

    # Define the function signature for the C++ function
    _call_java_static_method = _lib.call_java_static_method
    _call_java_static_method.argtypes = [
        ctypes.c_char_p, # class_name
        ctypes.c_char_p, # method_name
        ctypes.c_char_p, # signature
        ctypes.POINTER(Py2JibArg), # args
        ctypes.c_int      # arg_count
    ]
    _call_java_static_method.restype = None


def _get_signature(args):
    """Generates the JNI method signature from Python arguments."""
    sig_parts = []
    for arg in args:
        if isinstance(arg, str):
            sig_parts.append("Ljava/lang/String;")
        elif isinstance(arg, int):
            sig_parts.append("I")
        # Add other type mappings here (e.g., float -> F, bool -> Z)
        else:
            raise TypeError(f"Unsupported argument type: {type(arg)}")
    # For now, we assume a void return type.
    return f"({''.join(sig_parts)})V"


class JavaMethod:
    def __init__(self, class_name, method_name):
        self._class_name = class_name.replace('.', '/') # JNI uses / separators
        self._method_name = method_name

    def __call__(self, *args):
        if not _lib:
            raise RuntimeError("Py2Jib library not loaded. Call py2jib.init() first.")

        arg_count = len(args)
        # Create an array of our C structure
        args_array = (Py2JibArg * arg_count)()

        # Populate the array
        for i, arg in enumerate(args):
            if isinstance(arg, str):
                args_array[i].type = ArgType.TYPE_STRING
                args_array[i].s_val = arg.encode('utf-8')
            elif isinstance(arg, int):
                args_array[i].type = ArgType.TYPE_INT
                args_array[i].i_val = arg
            else:
                raise TypeError(f"Unsupported argument type: {type(arg)}")

        signature = _get_signature(args)

        _lib.call_java_static_method(
            self._class_name.encode('utf-8'),
            self._method_name.encode('utf-8'),
            signature.encode('utf-8'),
            args_array,
            arg_count
        )

class JavaClass:
    def __init__(self, class_name_parts):
        self._class_name_parts = class_name_parts

    def __getattr__(self, name):
        # If we get another attribute, it's a nested class
        if name[0].islower(): # It's a method
            class_name = '.'.join(self._class_name_parts)
            return JavaMethod(class_name, name)
        else: # It's a nested class name
            return JavaClass(self._class_name_parts + [name])

class JNI:
    def __getattr__(self, name):
        return JavaClass([name])

jni = JNI()
