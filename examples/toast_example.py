from py2jib.android import Toast
import time

print("--- Running Toast Example ---")

# In a real Android environment, this would show a native toast.
Toast.show("Hello from Python!", Toast.LENGTH_LONG)

print("Toast call issued. Check the device screen.")
time.sleep(2) # Keep script alive to see toast

print("--- Toast Example Finished ---")
