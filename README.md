# Voting Machine

A DIY voting device with three buttons - green, yellow and red.


## Requirements and set-up

- Download [Pololu Pushbutton](https://github.com/pololu/pushbutton-arduino) library for Arduino
- Download [ArduinoJson](https://github.com/bblanchon/ArduinoJson) library
- Set up Arduino IDE for NodeMCU development. A good tutorial is [here](http://henrysbench.capnfatz.com/henrys-bench/arduino-projects-tips-and-more/arduino-esp8266-lolin-nodemcu-getting-started/).
- Download and install [ESP8266 filesystem uploader](https://github.com/esp8266/arduino-esp8266fs-plugin). You will need it to upload static files (HTML, fonts etc) from `data` directory.
- Compile and upload the sketch and static files (use the `Tools -> ESP8266 sketch data upload` menu in Arduino IDE).
- Connect to the Access Point. Default name is `Votin' machine` and password is `thatwaseasy`.
- Configure the voting machine by pointing your browser to the URL http://192.168.1.1/config.
- View the current votes by going to http://192.168.1.1/results.html

## Useful links

[Arduino ESP8266 WiFi library reference](https://github.com/esp8266/Arduino/tree/master/doc/esp8266wifi)