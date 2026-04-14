# Plan of Attack for Kiwibots

## Testing of laptops and esp-32 clients
Place laptops outside, fully charged and run a data collection client. Saves As much data on the kiwibots as we can gather. Leave them out there attended with hopefully kiwibots passing by occasionally
### Data we can collect during it
* Mac Addresses of kiwibots
* Wifi SIgnals
* Time recordings as well to line up with the camera recording
* Camera Recording of the scene to look back to later
* Rough Location of laptop


## Math needed to do post data collection
Maybe ray calculations to find the intersection of circles? Rough calculations of range for laptops and/or esp-32 clients. Speed calculations for kiwibots to help with estimating where they are.

## Timeline Visualizer thoughts
Do I make config files for the kiwibots and do calculations each time it loads? Or should I have it run calculations once and write to separate files and assign an id to it?
How can I calculate a range? Where can I find a definitive wifi range? Or how can I get a definitive wifi range from a device? If I do that maybe config files would be good to assign a range to each device.
