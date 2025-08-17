from . import jni

# High-level wrappers for Android APIs

class Toast:
    # Constants from android.widget.Toast
    LENGTH_SHORT = 0
    LENGTH_LONG = 1

    _helper = jni.com.py2jib.ToastHelper

    @staticmethod
    def show(message, duration=LENGTH_SHORT):
        """Shows an Android Toast message by calling the Java helper."""
        Toast._helper.show(message, duration)

class WebView:
    _helper = jni.com.py2jib.WebViewHelper

    @staticmethod
    def run_js(script):
        """
        Runs JavaScript in the app's WebView.
        This is a mock call, the real implementation would call the Java method.
        """
        print(f"[WebView] Running JS: {script}")
        # In a real scenario:
        # self._helper.runJs(script)

class Sensor:
    _helper = jni.com.py2jib.SensorHelper

    @staticmethod
    def get_accelerometer_data():
        """
        Gets the latest accelerometer data.
        This is a mock call, the real implementation would call the Java method.
        """
        print("[Sensor] Getting accelerometer data")
        # In a real scenario:
        # return self._helper.getAccelerometerData()
        return [0.0, 0.0, 9.8] # Mock data
