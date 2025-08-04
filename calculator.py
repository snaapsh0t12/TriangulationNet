"""
Handles the calculating of position for the target when given which nodes can ping the target

Takes the node ids and returns a range of points where the target could be

"""

import csv
import matplotlib.pyplot as plt
import matplotlib.image as mpimg

def get_coordinates(node_id):
    # Takes a node id, looks in nodes.csv to return the coordinates for that id
    with open("nodes.csv", mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            if row[0] == node_id: 
                x = int(row[1])
                y = int(row[2])
                return (x, y)
    return None  # Return None if the node ID is not found

def possible_coordinates(detecting_nodes, csv_file='nodes.csv'):
    detected_coordinates = None  # To hold the intersection of possible coordinates
    nodes = {}

    # Load node data from CSV
    with open(csv_file, mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            node_id = row[0].strip()
            x = int(row[1].strip())
            y = int(row[2].strip())
            range_value = int(row[3].strip())
            nodes[node_id] = (x, y, range_value)

    # Find possible coordinates from detecting nodes
    for node_id in detecting_nodes:
        if node_id in nodes:
            x, y, range_value = nodes[node_id]
            current_coordinates = set()
            for dx in range(-range_value, range_value + 1):
                for dy in range(-range_value, range_value + 1):
                    if dx**2 + dy**2 <= range_value**2:  # Within circular range
                        current_coordinates.add((x + dx, y + dy))
            
            # Initialize or intersect with detected coordinates
            if detected_coordinates is None:
                detected_coordinates = current_coordinates
            else:
                detected_coordinates &= current_coordinates  # Intersection

    # Remove coordinates that fall within the range of non-detecting nodes
    non_detecting_nodes=[]

    for i in nodes:
        if i[0] not in detecting_nodes:
            non_detecting_nodes.append(i[0])
    
    for node_id in non_detecting_nodes:
        if node_id in nodes:
            x, y, range_value = nodes[node_id]
            for dx in range(-range_value, range_value + 1):
                for dy in range(-range_value, range_value + 1):
                    if dx**2 + dy**2 <= range_value**2:  # Within circular range
                        detected_coordinates.discard((x + dx, y + dy))

    # Plotting
    plt.figure(figsize=(10, 10))
    
    # Load and display the background image
    img = mpimg.imread('static/images/dark_map.png')
    plt.imshow(img, extent=[-20, 100, -20, 40])  # Adjust extent to fit your coordinates
    
    # Plot each node with its range
    for node_id, (x, y, range_value) in nodes.items():
        #circle = plt.Circle((x, y), range_value, color='blue', fill=False, linestyle='dashed')
        circle = plt.Circle((x, y), range_value, color='blue', fill=False, linestyle=(0, (1, 10))) #https://matplotlib.org/stable/gallery/lines_bars_and_markers/linestyles.html
        plt.gca().add_artist(circle)
        plt.plot(x, y, 'o', label=node_id)
        plt.text(x, y, f' {node_id}', fontsize=10, verticalalignment='bottom')  # Label next to node

    # Highlight the shared possible coordinates
    if detected_coordinates:
        x_coords, y_coords = zip(*detected_coordinates)
        print(x_coords, y_coords)
        plt.scatter(x_coords, y_coords, color='red', marker='x', label='Possible Coordinates')

    plt.xlim(-20, 100)  # Adjust limits as necessary
    plt.ylim(-20, 40)  # Adjust limits as necessary
    plt.title('Target Triangulation')
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.axhline(0, color='black', linewidth=0.5, ls='--')
    plt.axvline(0, color='black', linewidth=0.5, ls='--')
    plt.grid(color='gray', linestyle='--', linewidth=0.5)
    #plt.legend() #Enable this for the legend
    plt.savefig('static/images/image.png')
    plt.gca().set_aspect('equal', adjustable='box')
    #plt.show()

    if detected_coordinates is None:
        return None
    else:
        return list(detected_coordinates)

#detecting_nodes = ['node1', 'node2']
#non_detecting_nodes = ['node4', 'node3']
#possible_coordinates(detecting_nodes, non_detecting_nodes)

