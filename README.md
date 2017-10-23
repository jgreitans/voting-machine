# Voting Machine
A DIY voting device with three buttons - green, yellow and red.

## Requirements and set-up

### voting-machine-arduino
Pololu Pushbutton library for Arduino - see https://github.com/pololu/pushbutton-arduino

### voting-machine-nodemcu
Before compiling, you must create file `wifi-password.h` in `voting-machine-nodemcu` directory.
The file must contain definition of Wi-Fi SSID and password:
```
const char* ssid = "your-ssid";
const char* password = "your-password";
```
