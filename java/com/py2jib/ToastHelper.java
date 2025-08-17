package com.py2jib;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.widget.Toast;

public class ToastHelper {

    public static final int LENGTH_SHORT = Toast.LENGTH_SHORT;
    public static final int LENGTH_LONG = Toast.LENGTH_LONG;

    public static void show(final String message, final int duration) {
        final Context context = Py2Jib.getContext();
        if (context == null) return;

        new Handler(Looper.getMainLooper()).post(() -> Toast.makeText(context, message, duration).show());
    }
}
