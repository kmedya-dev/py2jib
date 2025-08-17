package com.py2jib;

import android.content.Context;

public class Py2Jib {

    static {
        System.loadLibrary("py2jib");
    }

    private static Context applicationContext;

    /**
     * Initializes the Py2Jib bridge. Must be called from the main thread
     * on app startup.
     * @param context The application context.
     */
    public static void init(Context context) {
        applicationContext = context.getApplicationContext();
        initBridge(); // Caches the JavaVM instance
    }

    /**
     * Gets the cached application context.
     * @return The application context.
     */
    public static Context getContext() {
        if (applicationContext == null) {
            throw new IllegalStateException("Py2Jib not initialized. Call Py2Jib.init(context) first.");
        }
        return applicationContext;
    }

    /**
     * Native method to initialize the C++ side of the bridge.
     */
    private static native void initBridge();
}
