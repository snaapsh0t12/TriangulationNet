"""
This file handles taking the data received at the server, processing it, and making the map and logs

It runs in a separate process as the server so that it can run with its own time delays

"""

import time

global config

def read_config():
    global config

    config = {}
    
    with open("config", 'r') as file:
        for line in file:
            # Strip leading/trailing whitespace and skip empty lines or comments
            line = line.strip()
            if line and not line.startswith("#"):  # Skips comments starting with '#'
                var_name, var_value = line.split(maxsplit=1)  # Split into name and value
                config[var_name] = var_value

def write_logs(data):
    print(data)

def write_map(data):
    print(data)

def process_cache():
    # Takes the cache, filters for data we need, and uses calculator.py to calculate positioning, then calls write_logs() and write_map()
    return "cache"

    open('cache', 'w').close()

    write_logs()
    write_map()


last_read_config = time.time()
read_config()

last_read_cache = time.time()
process_cache()

while True:

    if time.time() - last_read_config > int(config['config_update_wait']):
        read_config()

    if time.time() - last_read_cache > int(config['clock_delay']):
        process_cache()


