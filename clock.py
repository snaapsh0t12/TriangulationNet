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
    # Build a dict of the latest signal seen from each node
    latest_by_node = {}

    for line in cache:
        parts = line.strip().split()
        if len(parts) < 3:
            continue

        node_id = parts[0]
        strength = parts[1]

        try:
            timestamp = float(parts[2])
        except ValueError:
            continue

        if node_id not in latest_by_node or timestamp > latest_by_node[node_id]["timestamp"]:
            latest_by_node[node_id] = {
                "strength": strength,
                "timestamp": timestamp
            }

    detected_signals = {}
    for node_id, info in latest_by_node.items():
        detected_signals[node_id] = info["strength"]

    best_point = calculator.weighted_best_point(detected_signals)

    if best_point is None:
        point_text = "no_best_point"
    else:
        point_text = f"best_point: ({best_point[0]:.2f}, {best_point[1]:.2f})"

    with open("log.log", "a") as f:
        f.write(f"Time: {round(time.time())}\n")
        f.write(f"{point_text}\n")
        f.write(f"signals: {detected_signals}\n")
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


