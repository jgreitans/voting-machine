#include "server.h"
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "config.h"
#include "FS.h"

ESP8266WebServer server(80);
extern Configuration config;
extern unsigned long totalGreen;
extern unsigned long totalYellow;
extern unsigned long totalRed;
extern void reset(bool fullReset);

bool serveFile(String path);
void serveGetConfig();
void servePostConfig();
void serveNotFound();

void startWebServer() {
  server.on("/", HTTP_GET, []() { serveFile("/results.html"); });
  server.on("/results", HTTP_GET, []() { serveFile("/results.html"); });
  server.on("/votes", HTTP_GET, []() {
    String content = String("{") + "\"green\":" + String(totalGreen) + ", \"yellow\":" + String(totalYellow) + ", \"red\":" + String(totalRed) + "}";
    server.send(200, "application/json", content);
  });

  // config
  server.on("/config", HTTP_GET, serveGetConfig);
  server.on("/config", HTTP_POST, servePostConfig);

  // restart
  server.on("/restart", HTTP_GET, []() {
    String content = "<!doctype html><html><body><h1>&#x1F631; Restart?</h1><form action=\"\" method=\"post\"><input type=\"submit\" value=\"Restart! &#x1F47F;\"/></form></body></html>";
    server.send(200, "text/html", content);
  });
  server.on("/restart", HTTP_POST, []() {
    String content = String("<!doctype html><html><head><meta http-equiv=\"refresh\" content=\"5;url=") + WiFi.softAPIP() + "/\"/></head><body>&#x1F64F; Restarting. You wait here!</body></html>";
    server.send(200, "text/html", content);
    delay(250);
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    pinMode(15, OUTPUT);
    digitalWrite(15, LOW);
    ESP.restart();
  });

  server.on("/reset", HTTP_GET, []() { serveFile("/reset.html"); });
  server.on("/reset", HTTP_POST, []() {
    reset(true);
    server.sendHeader("Location", "/");
    server.send(303);
  });

  server.onNotFound([]() {
    if(!serveFile(server.uri())) {
      serveNotFound();
      }
  });

  server.begin();
}

void stopWebServer() {
  server.stop();
}

void processWebRequests() {
  server.handleClient();
}

void serveNotFound() {
  server.send(404, "text/html", "<!doctype html><html><body><h1>Page not found &#x1f61e;</h1></body></html>");
}

String getSsids() {
  String ssidList = "";
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    String currentSsid = WiFi.SSID(i);
    currentSsid.replace("\"", "&quot;");
    currentSsid.replace("<", "&lt;");
    ssidList += "<option value=\"" + currentSsid + "\"";
    if (WiFi.SSID(i).equals(WiFi.SSID())) {
      ssidList += "selected";
    }
    ssidList += ">" + currentSsid + "</option>";
  }
  return ssidList;
}

String getFileContents(String path){
  if(path.endsWith("/")) path += "index.html";
  if(SPIFFS.exists(path)){
    File file = SPIFFS.open(path, "r");
    String data = file.readString();
    file.close();
    return data;
  }
  return "";
}


void serveGetConfig() {
  String configPageContent = getFileContents("/config.html");
  if (configPageContent.length() == 0) {
    serveNotFound();
    return;
  }
  configPageContent.replace("{wifi-ssid-list}", getSsids());
  configPageContent.replace("{ap-ssid}", config.accessPointSsid);
  configPageContent.replace("{ap-password}", config.accessPointPassword);
  configPageContent.replace("{wifi-ssid}", config.wifiSsid);
  configPageContent.replace("{wifi-password}", config.wifiPassword);
  configPageContent.replace("{send-to-endpoint}", config.sendToEndpoint ? "checked" : "");
  configPageContent.replace("{endpoint}", config.targetEndpoint);
  configPageContent.replace("{apikey}", config.apiKey);
  configPageContent.replace("{timeout}", String(config.timeoutInMillis));
  configPageContent.replace("{local-ip}", WiFi.isConnected() ? WiFi.localIP().toString() : "<span class=\"warning\">not connected</span>");
  configPageContent.replace("{auto-connect}", config.autoConnectToWifi ? "checked" : "");
  server.send(200, "text/html", configPageContent);
}

void servePostConfig() {
  bool reconnect = false;
  String val = server.arg("ap-ssid");
  if (val) {
    reconnect = reconnect || !config.accessPointSsid.equals(val);
    config.accessPointSsid = val;
  }
  val = server.arg("ap-password");
  if (val) {
    reconnect = reconnect || !config.accessPointPassword.equals(val);
    config.accessPointPassword = val;
  }
  val = server.arg("wifi-ssid");
  if (val) {
    reconnect = reconnect || !config.wifiSsid.equals(val);
    config.wifiSsid = val;
  }
  val = server.arg("wifi-password");
  if (val) {
    reconnect = reconnect || !config.wifiPassword.equals(val);
    config.wifiPassword = val;
  }
  if (server.hasArg("send-to-endpoint")) {
    config.sendToEndpoint = server.arg("send-to-endpoint").equals("true");
  } else {
    config.sendToEndpoint = false;
  }
  val = server.arg("endpoint");
  if (val) config.targetEndpoint = val;
  val = server.arg("apikey");
  if (val) config.apiKey = val;
  val = server.arg("timeout");
  if (val) config.timeoutInMillis = val.toInt();

  config.save();

  String content = "<!doctype html><html><body><p>Configuration was updated &#x1F44C;<p><p><a href=\"/config\">Back</a></p></body></html>";
  server.send(200, "text/html", content);

  if (WiFi.isConnected() && reconnect) {
    while(WiFi.isConnected()) {
      WiFi.disconnect();
      delay(1);
    }
    WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());

    WiFi.softAPdisconnect();
    WiFi.softAP(config.accessPointSsid.c_str(), config.accessPointPassword.c_str());
  }
}

String getContentType(String filename){
  if(server.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".ttf")) return "application/x-font-truetype";
  else if(filename.endsWith(".json")) return "application/json";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".gz")) return "image/x-gzip";
  else if(filename.endsWith(".zip")) return "image/x-zip";
  return "text/plain";
}

bool serveFile(String path) {
  if(path.endsWith("/")) path += "index.html";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

