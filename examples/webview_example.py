from py2jib.android import WebView
import time

print("--- Running WebView Example ---")

# In a real Android environment, this would execute JS in a WebView.
WebView.run_js("console.log('Hello from Python-injected JavaScript!');")
WebView.run_js("document.body.style.backgroundColor = 'lightblue';")

print("JavaScript injection calls issued.")
time.sleep(2)

print("--- WebView Example Finished ---")
