"""    plt.figure(figsize=(10, 10))
    img = mpimg.imread('static/images/dark_map.png')
    plt.imshow(img, extent=[0, 100, 0, 40])

    for node in (node1, node2):
        circle = plt.Circle((node.x, node.y), node.range, color='blue', fill=False, linestyle=(0, (1, 10)))
        plt.gca().add_artist(circle)
        plt.plot(node.x, node.y, 'o', label=node.id)
        plt.text(node.x, node.y, f' {node.id}', fontsize=10, verticalalignment='bottom', color="white")

    plt.xlim(0, 100)
    plt.ylim(0, 40)
    
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.axhline(0, color='black', linewidth=0.5, linestyle='--')
    plt.axvline(0, color='black', linewidth=0.5, linestyle='--')
    plt.grid(color='gray', linestyle='--', linewidth=0.5)
    plt.gca().set_aspect('equal', adjustable='box')


    if active_node is None:
        print("No detection signal from either node.")
        plt.title(f'Target Detection (Target not found)')

    else:

        # Compute target position as an offset from the active node
        angle = math.radians(angle_degrees)
        target_x = active_node.x + active_node.range * math.cos(angle)
        target_y = active_node.y + active_node.range * math.sin(angle)

        # Mark the target position (red star)
        plt.plot(target_x, target_y, 'r*', markersize=15, label='Target')


    plt.savefig('static/images/image.png')
    plt.savefig('logs/images/image'+str(time.time())+'.png')
    plt.close()
"""