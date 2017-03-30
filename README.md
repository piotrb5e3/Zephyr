# Zephyr
Arduino client for Dyeus

# Requirements
* Python3
* [pySerial](https://pythonhosted.org/pyserial/pyserial.html#installation)
* [PlatformIO](http://docs.platformio.org/en/stable/installation.html)
* Eclipse IDE (optionally) 

# Setup
* Clone this repository
* Run `platformio init --ide eclipse` in `app` and `configurator`
* Import `app`, `configurator` and `common` into eclipse

# Building
`platformio run`
OR
build from eclipse

# Upload to board
`platformio run -t upload`
OR
run upload target from eclipse
