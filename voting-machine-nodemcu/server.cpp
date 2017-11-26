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

bool serveFile(String path);
void serveResults();
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

  server.onNotFound([]() {
    if(!serveFile(server.uri())) {
      serveNotFound();;
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

void serveResults() {
  String contents = "<!doctype html><html><head><link href=\"https://fonts.googleapis.com/css?family=Frijole|Piedra|Finger+Paint&amp;subset=latin-ext\" rel=\"stylesheet\"><style>body{font-family:'Finger Paint',cursive;font-size:x-large}.luxofor{position:fixed;width:800px;height:300px;top:50%;left:50%;margin:-150px 0 0 -400px}.green,.yellow,.red{width:250px;height:250px;line-height:250px;color:black;font-family:'Frijole',cursive;font-size:96px;border-radius:100%;text-align:center}.green{background-color:green}.yellow{background-color:yellow}.red{background-color:red}.panel{display:inline-block}.value{vertical-align:middle}.description{text-align:center;margin-top:50px}</style></head><body><div class=\"luxofor\"><div class=\"green panel\"> <span id=\"green\" class=\"value\">000</span></div><div class=\"yellow panel\"> <span id=\"yellow\" class=\"value\">000</span></div><div class=\"red panel\"> <span id=\"red\" class=\"value\">000</span></div></div><p class=\"description\">This is the current number of votes since the voting machine was restarted.</p> <script type=\"text/javascript\">function processResponse(){var json=JSON.parse(this.responseText);document.getElementById(\"green\").innerHTML=json.green;document.getElementById(\"yellow\").innerHTML=json.yellow;document.getElementById(\"red\").innerHTML=json.red;} function processError(){console.error(\"Could not get votes.\");document.getElementById(\"green\").innerHTML=\"-1\";document.getElementById(\"yellow\").innerHTML=\"-1\";document.getElementById(\"red\").innerHTML=\"-1\";} function sendRequest(){var xhr=new XMLHttpRequest();xhr.addEventListener(\"load\",processResponse);xhr.addEventListener(\"error\",processError);xhr.open(\"GET\",\"/votes\",true);xhr.send();} setInterval(sendRequest,5000);</script> </body></html>";
  server.send(200, "text/html", contents);
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
  configPageContent.replace("{local-ip}", WiFi.localIP().toString());
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

  if (reconnect) {
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

