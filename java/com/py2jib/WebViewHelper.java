package com.py2jib;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.webkit.JavascriptInterface;
import android.webkit.WebView;
import android.webkit.WebViewClient;
import android.widget.Toast;

public class WebViewHelper {

    private static WebView webView;

    public static void createWebView(final Context context) {
        new Handler(Looper.getMainLooper()).post(() -> {
            webView = new WebView(context);
            webView.getSettings().setJavaScriptEnabled(true);
            webView.setWebViewClient(new WebViewClient());
            webView.addJavascriptInterface(new WebAppInterface(), "Android");
            // Add the webview to the current view hierarchy, this part is application-dependent
            // For a simple test, we are not adding it to any layout.
        });
    }

    public static void loadUrl(final String url) {
        if (webView == null) return;
        new Handler(Looper.getMainLooper()).post(() -> webView.loadUrl(url));
    }

    public static void runJs(final String jsCode) {
        if (webView == null) return;
        new Handler(Looper.getMainLooper()).post(() -> webView.evaluateJavascript(jsCode, null));
    }

    public static class WebAppInterface {
        @JavascriptInterface
        public void showToast(String toast) {
            Toast.makeText(Py2Jib.getContext(), toast, Toast.LENGTH_SHORT).show();
        }

        @JavascriptInterface
        public void log(String message) {
            // In a real app, you'd pipe this to your Python layer
            System.out.println("JS Log: " + message);
        }
    }
}
