# Triangulation

This is the code for a research project. It's goal is to determine how well an attacker can track a live target by using the bluetooth signals that the target's everyday electronics give off (smartwatches, earbuds, etc). There will be many nodes placed in an area which report whether they can detect the target mac address to a central computer. The central computer then uses that to triangulate where the target is on a map and track their real-time location.

## Running

If you want to run it, clone the repo, create a python virtual environment from the requirements file with `python -m venv env` and then `pip install -r requirments.txt`, and then run it with `python3 server.py`

This will start the server, there's a basic cline tin client.py which registers a node

**This software is in no way stable**

## TODO

- Adjust to wifi for kiwi bots

## Parts

### Nodes

The nodes are small Raspberry Pi Zero Ws. They are placed around an area. When they can detect (ping) the target mac address they send a notification to the central server.

When booted up, it sends a notification to the server and retrieves configurations

Note: They do not recheck and send a new notification every x seconds, they only send a notification when they can sense the target, they lie in wait otherwise

To ping, the node uses {that blueoth hcitool line} and scans the output for the target_address. If it finds the address, sends a notifications to the server with the node's id mac address and the target's target_address (because the config may have changed since the last time they retrieved the config and a new target_address may be in use (if the address is wrong, the server will return a response saying that the node needs to update the config))

The nodes also recheck the server's config file every config_update_wait seconds (they request the hash from the server and if different from the local one redownloads)

### Central Server

The central server (laptop) connects to the nodes over wifi and uses the location they are placed in to the target to triangulate the position of the target.

It also hosts a form to hot swap the configs (so the user can change the target address midway through) and/or nodes database (coordinates). To do this it also needs to have a page which returns the current hash of the config

When a node reaches out to the server for registration the server pings the user's open dashboard and tells them to input coordinates for the node

If a node sends a ping to the central server with the wrong target_address, the server responds with an error which tells the node to update its config

The cache is in a text file. When the server receives a sighting confirmation it writes the node which saw it and the time. The logic for deciding how old a sighting can be for it to be credible is handled by the clock

When a node is registered its added to the cache with a last sighting timestamp of 0

The clock reloads the config every run to get the newest clock_delay

## Process

Each node has has an id of their mac address. All communication from each node is accompanied by it's mac address (ip changes, ids rotate on reboot). Its up to the maintainer to keep the list of mac addresses and associated coordinates.

server.py simply hosts image.png and accepts info from nodes (saved into a cache). clock.py does the real work, it takes the cache from the server with which nodes can ping the target, and uses calculator.py to rewrite image.png with the updated map. clock.py is also where logic which compensates for real world conditions is located (eg. rebooting nodes). If a node has to reboot it sends a notification to the server and is left out of both detections and non detections lists, essentially it is removed from the network until booted up again (when its booted up it sends a notification)

clock.py will also have to determine when a node which has pinged the thing
server.py and clock.py will have to run at the same time

### config

The config file provides an easy interface to test different settings and distribute to the many followers with ease. When nodes boot up they read the config and adjust

- target_address (string)
  - A regex string for the target's bluetooth or wifi mac address, can be a comma-separate list of targets
- detection_method (wifi/bluetooth)
  - Whether to use bluetooth or wifi for target detection
- le (bool)
  - Whether to scan le or just scan on the nodes (only when detection_method=bluetooth)
- config_update_wait (int)
  - The number of seconds before the nodes should re-check the configs on the server
- clock_delay (int)
  - The number of seconds the clock should wait before reloading the map
- range (x, x, y, y)
  - The bounds for the search grid
- offline (bool)
  - Whether the ping signatures should be reported to a central server. Some situations might not allow the node to send the pings over a network, so the pings will be saved in a file specified in offline_report_file (default is ./logs)
- offline_report_file (file/path/string)
  - The file path to the report that offline logs should be saved to if offline is set to True

## Developing

### Technical Overview

server.py holds the server which runs on the laptop, client.py holds my progress towards the client which would be on a node.

The html is held under /templates, index.html is the main dashboard. The css, js, and images is kept under /static, /static/images/image.png is the map which is constantly redrawn by calculator.py (more on this later) and served on the dashboard. map.png is the blank map.

calculator.py holds the functions which take the nodes which can detect the target, their ranges, and returns an image with the possible area for the target to be in.

clock.py is the py file which waits for x seconds, an then takes the cache created by server.py and uses calculator.py to generate the image. clock.py is also the place where logic for compensating for real world conditions will be.

### Todo

- Add an option on the dashboard which has the list of nodes and their ips, and have a button for each one which pings them to see if they're online and running
- Make the ping function tell the node to re register if its not already registered
- Add offline function to save logs to an offline file which can be collected and correlated later
- Make calculations take signal strength into account
- Make calculations take multiple targets into account and add tracking to map
- Add a way to give nodes an ip endpoint for the central server
- Add a way to save the images for different times and compress them into a video for a time lapse or non time lapse playback