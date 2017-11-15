# Voting Machine

A DIY voting device with three buttons - green, yellow and red.


## Requirements and set-up

Pololu Pushbutton library for Arduino - see https://github.com/pololu/pushbutton-arduino

### voting-machine-arduino

No specific requirements.

### voting-machine-nodemcu

Set up Arduino IDE for NodeMCU development. A good tutorial is [here](http://henrysbench.capnfatz.com/henrys-bench/arduino-projects-tips-and-more/arduino-esp8266-lolin-nodemcu-getting-started/)

Before compiling, you must create file `wifi-password.h` in `voting-machine-nodemcu` directory.
The file must contain definition of Wi-Fi SSID and password and Access Point name and password (these will
be used to allow to configure the voting machine by connecting to the AP and opening browser URL http://192.168.1.1/config):
```
const char* ssid = "your-ssid";
const char* password = "your-password";
const char* accessPointSsid = "AP name";
const char* accessPointPassword = "ap-password";
```


## Useful links

[Arduino ESP8266 WiFi library reference](https://github.com/esp8266/Arduino/tree/master/doc/esp8266wifi)