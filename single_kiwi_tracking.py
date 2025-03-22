"""
This file is only for the single kiwibot proof of concept, intended to draw the map with the location of that one kiwibot which travels between the lab and farther down VLL

Therefore, it only handles a single target detected by a single node, so it just averages the location of the bot when detected by a node

Range in this case will be feet. It should be just plain feet but the nodes will be inside so that complicates things, we'll probably just manually measure it with the nodes inside and adjust accordingly
"""

class Node:
  def __init__(self, name, id, x, y, range, signal_detected=False):
    self.name = name
    self.id = id
    self.x = x
    self.y = y
    self.range = range
    self.signal_detected = False

node1 = Node("Lab node", "5a:84:f7:9b:1f:09", 1, 1, 15)
node2 = Node("VLL node", "mac_address", 10, 10, 15)

import math
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

# Example Node class

def update_nodes_from_cache(node1, node2, cache_file='cache.txt'):
    """
    Reads the cache file to determine which node last detected the target.
    Each line in the file is in the format: "id strength timestamp".
    The node with the most recent timestamp is marked as detecting the target.
    """
    signals = []
    try:
        with open(cache_file, 'r') as f:
            for line in f:
                parts = line.strip().split()
                if len(parts) < 3:
                    continue  # Skip any improperly formatted lines
                id = parts[0]
                strength = float(parts[1])
                timestamp = float(parts[2])
                signals.append((id, strength, timestamp))
    except FileNotFoundError:
        print(f"Cache file '{cache_file}' not found.")
        return

    if not signals:
        print("No valid signals found in cache.")
        return

    # Determine the most recent signal by timestamp
    signals.sort(key=lambda s: s[2], reverse=True)
    latest_signal = signals[0]
    id = latest_signal[0]

    # Reset detection status and mark the active node
    node1.signal_detected = (node1.id == id)
    node2.signal_detected = (node2.id == id)

    print(f"Target detected by {id} at timestamp {latest_signal[2]} with strength {latest_signal[1]}.")

def plot_target(node1, node2, angle_degrees=45):
    """
    Plots the nodes and draws the target at a distance equal to the node's range
    in the direction specified by angle_degrees from the active node.
    """
    # Determine which node detected the target
    active_node = None
    for node in (node1, node2):
        if node.signal_detected:
            active_node = node
            break

    if active_node is None:
        print("No detection signal from either node.")
        return

    # Compute target position as an offset from the active node
    angle = math.radians(angle_degrees)
    target_x = active_node.x + active_node.range * math.cos(angle)
    target_y = active_node.y + active_node.range * math.sin(angle)

    # Set up the plot with a background image
    plt.figure(figsize=(10, 10))
    img = mpimg.imread('static/images/map.png')
    plt.imshow(img, extent=[-20, 100, -20, 40])  # Adjust extent as needed

    # Plot each node and its detection range circle
    for node in (node1, node2):
        circle = plt.Circle((node.x, node.y), node.range, color='blue', fill=False, linestyle=(0, (1, 10)))
        plt.gca().add_artist(circle)
        plt.plot(node.x, node.y, 'o', label=node.id)
        plt.text(node.x, node.y, f' {node.id}', fontsize=10, verticalalignment='bottom')

    # Mark the target position (red star)
    plt.plot(target_x, target_y, 'r*', markersize=15, label='Target')

    # Additional plot settings
    plt.xlim(-20, 100)
    plt.ylim(-20, 40)
    plt.title(f'Target Detection (Active Node: {active_node.id})')
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.axhline(0, color='black', linewidth=0.5, linestyle='--')
    plt.axvline(0, color='black', linewidth=0.5, linestyle='--')
    plt.grid(color='gray', linestyle='--', linewidth=0.5)
    plt.gca().set_aspect('equal', adjustable='box')
    plt.savefig('static/images/image.png')
    #plt.show()


while True:
    # Update node signal status based on the latest cache data
    update_nodes_from_cache(node1, node2, cache_file='cache')

    # Plot the nodes and the computed target location
    plot_target(node1, node2, angle_degrees=45)