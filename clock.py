"""
This file handles taking the data received at the server, processing it, and making the map and logs

It runs in a separate process as the server so that it can run with its own time delays

"""

import time
import calculator
import csv

def read_config():

    config = {}
    
    with open("config", 'r') as file:
        for line in file:
            # Strip leading/trailing whitespace and skip empty lines or comments
            line = line.strip()
            if line and not line.startswith("#"):  # Skips comments starting with '#'
                var_name, var_value = line.split(maxsplit=1)  # Split into name and value
                config[var_name] = var_value

    return config

def write_logs(cache):
    # Write the strength for each node, the coordinates that the target is in, and the time to the logs
    detected_nodes = []
    
    for line in cache:
        detected_nodes.append(line.split()[0])
    
    print(detected_nodes)

    coordinates = calculator.possible_coordinates(detected_nodes) # This gets all possible coordinates in range of all of the nodes, not using fancy math to determine the best point between them  # This function also maps the coordinates to the map

    if coordinates is None:
        coordinates = "no_coordinates"

    with open("log.log", "a") as f:
        f.write(f"Time: {round(time.time())}\n")
        f.write(f"{coordinates}\n")
        # print("STRENGTHS", strengths)
        f.writelines(detected_nodes)
        f.write("\n---\n")

def process_cache():
    # Takes the cache, filters for data we need, and uses calculator.py to calculate positioning, then calls write_logs()
    
    with open("data/cache", "r") as f:
        cache = f.readlines()

    # open('cache', 'w').close()

    write_logs(cache)


last_read_config = time.time()
config = read_config()

last_read_cache = time.time()
process_cache()

while True:

    if time.time() - last_read_config > int(config['config_update_wait']):
        # Update the config

        config = read_config()

    if time.time() - last_read_cache > int(config['clock_delay']):
        # Read the cache and pass to calculator

        process_cache()


