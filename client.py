"""
The code for the client nodes

TODO:
    - Needs to get the config from the server
    - Needs to register itself with its mac address
    - Needs to ping the server with signatures
    - Needs to reload the config every config_update_wait seconds
"""

import subprocess
import re
import time
import requests
from getmac import get_mac_address
mac_address = get_mac_address()

url = 'http://127.0.0.1:5000/'

# Register the node
registration = {'id': mac_address}
x = requests.post(url+"/register", json = registration)

def get_config(central_server_endpoint):

    config = (requests.get(central_server_endpoint+"config").text)

    with open("config", "w") as file:
        file.write(config)

def rescan_wifi():
    # Forces nmcli to rescan the available networks to get an updated profile of surrounding signals, otherwise it uses a cache for some time
    try:
        subprocess.run(['nmcli', 'device', 'wifi', 'rescan'], check=True)
    except subprocess.CalledProcessError as e:
        print("Error rescanning Wi-Fi networks:", e)

def get_wifi_networks():
    # Gets available Wifi SSIDs and signal strength using nmcli
    try:
        result = subprocess.run(['nmcli', '-t', '-f', 'SSID,SIGNAL', 'dev', 'wifi'], capture_output=True, text=True, check=True)
        networks = [line.split(':') for line in result.stdout.strip().split('\n')]
        return [(ssid, signal) for ssid, signal in networks if ssid]  # Remove empty SSIDs
    except subprocess.CalledProcessError as e:
        print("Error fetching Wi-Fi networks:", e)
        return []

def filter_networks(networks, pattern):
    # Filters the found networks using the target_address regex string
    regex = re.compile(pattern)
    return [(ssid, signal) for ssid, signal in networks if regex.search(ssid)]

def main():

    config = {}
    
    with open("config", 'r') as file:
        for line in file:
            # Strip leading/trailing whitespace and skip empty lines or comments
            line = line.strip()
            if line and not line.startswith("#"):  # Skips comments starting with '#'
                var_name, var_value = line.split(maxsplit=1)  # Split into name and value
                config[var_name] = var_value

    print(config)
    pattern = config['target_address']
    if pattern == "none":
        pattern = False
    
    while True:
        rescan_wifi()
        networks = get_wifi_networks()
        #print(networks)


        if not networks:
            print("No Wi-Fi networks found.")
        else:
            print("Sending Available Networks")

            if pattern: 
                networks = filter_networks(networks, pattern)
            
            for ssid, signal in networks:

                ping = {'id': mac_address, 'ssid': ssid, 'strength': signal}
                x = requests.post(url+"/ping", json = ping)

                print(f"SSID: {ssid}, Signal Strength: {signal}%")
        
        time.sleep(1)
        
if __name__ == "__main__":
    get_config(url)
    main()
