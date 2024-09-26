"""Find info for a bluetooth device with bluetoothctl"""

import subprocess
import time

"""
MACs:
80:E1:27:9D:53:6E - Flipper Inolin
24:95:2F:AB:2O:CD - Pixel Buds
DC:E5:5B:2B:65:9D - Pixel


"""
def get_bluetooth_info():
    # Run the bluetoothctl info command
    result = subprocess.run(['bluetoothctl', 'info', '24:95:2F:AB:2O:CD'], 
                            stdout=subprocess.PIPE, 
                            stderr=subprocess.PIPE, 
                            text=True)
    return result.stdout

def extract_rssi(info):
    # Find the line that contains 'RSSI:'
    for line in info.splitlines():
        if "RSSI:" in line:
            return line
    return None

def main():
    while True:
        try:
            info = get_bluetooth_info()

            #print(info)
            if "not available" in info and "DeviceSet" not in info:
                print("Not available")
                break

            rssi_line = extract_rssi(info)
            if rssi_line:
                print(rssi_line)
            else:
                print("Device not available.")
        except Exception as e:
            print(f"An error occurred: {e}")

        # Wait for 1 second before the next iteration
        time.sleep(1)

if __name__ == "__main__":
    main()
