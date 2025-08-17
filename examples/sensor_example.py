from py2jib.android import Sensor
import time

print("--- Running Sensor Example ---")

for i in range(5):
    # In a real environment, this would fetch live sensor data.
    data = Sensor.get_accelerometer_data()
    print(f"Accelerometer data: X={data[0]:.2f}, Y={data[1]:.2f}, Z={data[2]:.2f}")
    time.sleep(1)

print("--- Sensor Example Finished ---")
