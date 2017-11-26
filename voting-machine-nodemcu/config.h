#ifndef __config_h__
#define __config_h__

#include <cstring>
#include <ArduinoJson.h>
#include "FS.h"

#define BUFFER_SIZE JSON_OBJECT_SIZE(8) + 490

struct Configuration {
  String wifiSsid;
  String wifiPassword;
  String accessPointSsid;
  String accessPointPassword;
  String targetEndpoint;
  String apiKey;
  unsigned long timeoutInMillis;
  bool sendToEndpoint;

  Configuration()
  {
    wifiSsid = "";
    wifiPassword = "";
    accessPointSsid = "Votin' gadget";
    accessPointPassword = "thatwaseasy";
    targetEndpoint = "http://192.168.1.100:65367/api/votes";
    apiKey = "3db17c25b53248a2be53adafd98763b6";
    timeoutInMillis = 200;
    sendToEndpoint = true;
  }

  bool load() {
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
      return false;
    }

    size_t size = configFile.size();
    if (size > 1024) {
      return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);
  
    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);

    DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);
    JsonObject& json = jsonBuffer.parseObject(buf.get());
    if (!json.success()) {
      return false;
    }

    wifiSsid = (const char*)json["wifiSsid"];
    wifiPassword = (const char*)json["wifiPassword"];
    accessPointSsid = (const char*)json["accessPointSsid"];
    accessPointPassword = (const char*)json["accessPointPassword"];
    targetEndpoint = (const char*)json["targetEndpoint"];
    apiKey = (const char*)json["apiKey"];
    timeoutInMillis = json["timeoutInMillis"];
    sendToEndpoint = json["sendToEndpoint"];
  }

  bool save() {
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      return false;
    }
    printTo(configFile);
    return true;
  }

  void printTo(Print& print) {
    DynamicJsonBuffer jsonBuffer(BUFFER_SIZE);
    JsonObject& json = jsonBuffer.createObject();
    json["wifiSsid"] = wifiSsid;
    json["wifiPassword"] = wifiPassword;
    json["accessPointSsid"] = accessPointSsid;
    json["accessPointPassword"] = accessPointPassword;
    json["targetEndpoint"] = targetEndpoint;
    json["apiKey"] = apiKey;
    json["timeoutInMillis"] = timeoutInMillis;
    json["sendToEndpoint"] = sendToEndpoint;
    json.printTo(print);
  }

};

#endif
