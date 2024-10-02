# Triangulation

This is the code for a research project. It's goal is to determine how well an attacker can track a live target by using the bluetooth signals that the target's everyday electronics give off (smartwatches, earbuds, etc). There will be many nodes placed in an area which report whether they can detect the target mac address to a central computer. The central computer then uses that to triangulate where the target is on a map and track their real-time location.

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
  - The target's bluetooth mac address
- le (bool)
  - Whether to scan le or just scan on the nodes
- config_update_wait (int)
  - The number of seconds before the nodes should re-check the configs on the server
- clock_delay (int)
  - The number of seconds the clock should wait before reloading the map
- range (x, x, y, y)
  - The bounds for the search grid

### Todo

- Add an option on the dashboard which has the list of nodes and their ips, and have a button for each one which pings them to see if they're online and running
- Make the ping function tell the node to re register if its not already registered
