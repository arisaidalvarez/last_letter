## Using `last_letter` as the Physics Simulator for ArduPlane SITL

Follow the [ArduPlane SITL instructions](http://dev.ardupilot.com/wiki/setting-up-sitl-on-linux/) to download and compile the ArduPlane code, but instead of using their official repo, you will need to clone this fork:
```
https://github.com/Georacer/last_letter.git
```
to enable `last_letter` integration.

When everything is done, enter the ArduPlane directory, usually using `cd ~/ardupilot/ArduPlane`
and execute the SITL, using the external simulator:
```
sim_vehicle.sh -e --console
```

This will run the ArduPlane code, MAVProxy and `last_letter` along with RViz all in one go.
If `last_letter` communicates with ArduPlane correctly, MAVProxy should display 3D satellite fix and track 10 satellites.

You can issue commands or rc overrides using MAVProxy normally.

[back to table of contents](../../../README.md)