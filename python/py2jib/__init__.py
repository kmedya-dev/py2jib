import ctypes
import os

# --- CTypes Definitions ---

# This enum must be kept in sync with c_wrapper/py2jib.h
class ArgType:
    TYPE_VOID = 0
    TYPE_INT = 1
    TYPE_STRING = 2
    TYPE_FLOAT = 3
    TYPE_BOOLEAN = 4
    TYPE_LONG = 5
    TYPE_INT_ARRAY = 6
    TYPE_STRING_ARRAY = 7

# This structure must be kept in sync with c_wrapper/py2jib.h
class Py2JibArg(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int),
                ("i_val", ctypes.c_int),
                ("s_val", ctypes.c_char_p),
                ("f_val", ctypes.c_float),
                ("b_val", ctypes.c_bool),
                ("l_val", ctypes.c_longlong),
                ("int_array_val", ctypes.POINTER(ctypes.c_int)),
                ("string_array_val", ctypes.POINTER(ctypes.c_char_p)),
                ("array_size", ctypes.c_int)]

# This structure must be kept in sync with c_wrapper/py2jib.h
class Py2JibReturn(ctypes.Structure):
    _fields_ = [("type", ctypes.c_int),
                ("i_val", ctypes.c_int),
                ("s_val", ctypes.c_char_p),
                ("f_val", ctypes.c_float),
                ("b_val", ctypes.c_bool),
                ("l_val", ctypes.c_longlong),
                ("int_array_val", ctypes.POINTER(ctypes.c_int)),
                ("string_array_val", ctypes.POINTER(ctypes.c_char_p)),
                ("array_size", ctypes.c_int)]

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
    _call_java_static_method.restype = Py2JibReturn


def _get_signature(args):
    """Generates the JNI method signature from Python arguments."""
    sig_parts = []
    for arg in args:
        if isinstance(arg, str):
            sig_parts.append("Ljava/lang/String;")
        elif isinstance(arg, int):
            # Differentiate between int and long
            if arg > 2147483647 or arg < -2147483648: # Max/min 32-bit signed int
                sig_parts.append("J") # long
            else:
                sig_parts.append("I") # int
        elif isinstance(arg, float):
            sig_parts.append("F") # float
        elif isinstance(arg, bool):
            sig_parts.append("Z") # boolean
        elif isinstance(arg, list):
            if not arg: # Empty list, assume Object array or specific type if known
                sig_parts.append("[Ljava/lang/Object;")
            else:
                first_elem = arg[0]
                if isinstance(first_elem, int):
                    sig_parts.append("[I") # int array
                elif isinstance(first_elem, str):
                    sig_parts.append("[Ljava/lang/String;") # String array
                else:
                    raise TypeError(f"Unsupported array element type: {type(first_elem)}")
        else:
            raise TypeError(f"Unsupported argument type: {type(arg)}")
    # For now, we assume a void return type for methods that don't return anything.
    # The actual return type will be handled by the C++ layer.
    return f"({''.join(sig_parts)})V"


class JavaMethod:
    def __init__(self, class_name, method_name):
        self._class_name = class_name.replace('.', '/') # JNI uses / separators
        self._method_name = method_name

    def __call__(self, *args):
        if not _lib:
            raise RuntimeError("Py2Jib library not loaded. Call py2jib.init() first.")

        arg_count = len(args)
        args_array = (Py2JibArg * arg_count)()

        # Populate the array
        for i, arg in enumerate(args):
            if isinstance(arg, str):
                args_array[i].type = ArgType.TYPE_STRING
                args_array[i].s_val = arg.encode('utf-8')
            elif isinstance(arg, int):
                if arg > 2147483647 or arg < -2147483648:
                    args_array[i].type = ArgType.TYPE_LONG
                    args_array[i].l_val = arg
                else:
                    args_array[i].type = ArgType.TYPE_INT
                    args_array[i].i_val = arg
            elif isinstance(arg, float):
                args_array[i].type = ArgType.TYPE_FLOAT
                args_array[i].f_val = arg
            elif isinstance(arg, bool):
                args_array[i].type = ArgType.TYPE_BOOLEAN
                args_array[i].b_val = arg
            elif isinstance(arg, list):
                args_array[i].array_size = len(arg)
                if not arg: # Empty list
                    # Handle as empty array, type will be determined by signature
                    pass
                else:
                    first_elem = arg[0]
                    if isinstance(first_elem, int):
                        args_array[i].type = ArgType.TYPE_INT_ARRAY
                        c_array = (ctypes.c_int * len(arg))(*arg)
                        args_array[i].int_array_val = c_array
                    elif isinstance(first_elem, str):
                        args_array[i].type = ArgType.TYPE_STRING_ARRAY
                        # Encode strings to bytes and create a C array of char pointers
                        encoded_strings = [s.encode('utf-8') for s in arg]
                        c_array = (ctypes.c_char_p * len(encoded_strings))(*encoded_strings)
                        args_array[i].string_array_val = c_array
                    else:
                        raise TypeError(f"Unsupported array element type: {type(first_elem)}")
            else:
                raise TypeError(f"Unsupported argument type: {type(arg)}")

        signature = _get_signature(args)

        result = _lib.call_java_static_method(
            self._class_name.encode('utf-8'),
            self._method_name.encode('utf-8'),
            signature.encode('utf-8'),
            args_array,
            arg_count
        )

        # Process return value
        if result.type == ArgType.TYPE_VOID:
            return None
        elif result.type == ArgType.TYPE_INT:
            return result.i_val
        elif result.type == ArgType.TYPE_STRING:
            s_val = result.s_val.decode('utf-8')
            _lib.free_string(result.s_val) # Free memory allocated in C++
            return s_val
        elif result.type == ArgType.TYPE_FLOAT:
            return result.f_val
        elif result.type == ArgType.TYPE_BOOLEAN:
            return result.b_val
        elif result.type == ArgType.TYPE_LONG:
            return result.l_val
        elif result.type == ArgType.TYPE_INT_ARRAY:
            py_list = [result.int_array_val[j] for j in range(result.array_size)]
            _lib.free_int_array(result.int_array_val) # Free memory allocated in C++
            return py_list
        elif result.type == ArgType.TYPE_STRING_ARRAY:
            py_list = []
            for j in range(result.array_size):
                s_val = result.string_array_val[j].decode('utf-8')
                _lib.free_string(result.string_array_val[j]) # Free each string
                py_list.append(s_val)
            _lib.free_string_array_ptr(result.string_array_val) # Free the array of pointers
            return py_list
        else:
            raise RuntimeError(f"Unsupported return type from C++: {result.type}")

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
