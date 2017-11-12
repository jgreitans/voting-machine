# Voting Machine

A DIY voting device with three buttons - green, yellow and red.


## Requirements and set-up

Pololu Pushbutton library for Arduino - see https://github.com/pololu/pushbutton-arduino

### voting-machine-arduino

No specific requirements.

### voting-machine-nodemcu

Set up Arduino IDE for NodeMCU development. A good tutorial is [here](http://henrysbench.capnfatz.com/henrys-bench/arduino-projects-tips-and-more/arduino-esp8266-lolin-nodemcu-getting-started/)

Before compiling, you must create file `wifi-password.h` in `voting-machine-nodemcu` directory.
The file must contain definition of Wi-Fi SSID and password:
```
const char* ssid = "your-ssid";
const char* password = "your-password";
```
