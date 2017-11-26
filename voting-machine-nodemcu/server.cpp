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

void serveResults();
void serveGetConfig();
void servePostConfig();
void serveNotFound();

void startWebServer() {
  server.on("/", serveResults);

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
    delay(100);
    //
    pinMode(0, OUTPUT);
    digitalWrite(0, HIGH);
    pinMode(2, OUTPUT);
    digitalWrite(2, HIGH);
    pinMode(15, OUTPUT);
    digitalWrite(15, LOW);
    ESP.restart();
  });

  // votes
  server.on("/votes", HTTP_GET, []() {
    String content = String("{") +
      "\"green\":" + String(totalGreen) + "," +
      "\"yellow\":" + String(totalYellow) + "," +
      "\"red\":" + String(totalRed) +
    "}";
    server.send(200, "application/json", content);
  });

  server.on("/results", HTTP_GET, serveResults);

  server.onNotFound(serveNotFound);

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

void serveGetConfig() {
  // Content was minified (http://minifycode.com/html-minifier/) and escaped (https://www.freeformatter.com/java-dotnet-escape.html). For original content, see config-page.html in the source directory
  String configPageContent = "<!doctype html><html><head><meta charset=\"utf-8\"><title>Configure the Voting machine</title><style>.row{margin-bottom:0.5em;margin-top:0.5em}form{padding:1em}fieldset{padding:0.5em;margin-top:0.5em;margin-bottom:0.5em;border:1px solid lightgray}.col-label{display:inline-block;width:7em;text-align:right;margin-right:0.25em}.col-input{display:inline-block;min-width:15em}.col-input>*{width:100%;margin:0}</style></head><body><form action=\"\" method=\"post\"><fieldset><legend>&#x1F4E1; Access point</legend><div class=\"row\"><div class=\"col-label\"> <label for=\"ap-ssid\">AP SSID:</label></div><div class=\"col-input\"> <input id=\"ap-ssid\" name=\"ap-ssid\" type=\"text\" value=\"{ap-ssid}\" required=\"required\" maxlength=\"32\"></input></div></div><div class=\"row\"><div class=\"col-label\"> <label for=\"ap-password\">Password:</label></div><div class=\"col-input\"> <input id=\"ap-password\" name=\"ap-password\" type=\"password\" value=\"{ap-password}\" maxlength=\"32\"></input></div></div></fieldset><fieldset><legend>&#x1f4f6; Wi-Fi</legend><div class=\"row\"><div class=\"col-label\"> <label for=\"wifi-ssid\">SSID:</label></div><div class=\"col-input\"> <select id=\"wifi-ssid\" name=\"wifi-ssid\"> {wifi-ssid-list} </select></div></div><div class=\"row\"><div class=\"col-label\"> <label for=\"wifi-password\">Password:</label></div><div class=\"col-input\"> <input id=\"wifi-password\" name=\"wifi-password\" type=\"password\" value=\"{wifi-password}\" maxlength=\"32\"></input></div></div></fieldset><fieldset><legend>&#x1F517; Other</legend><div class=\"row\"><div class=\"col-label\"> <label for=\"endpoint\">Voting endpoint:</label></div><div class=\"col-input\"> <input id=\"endpoint\" name=\"endpoint\" type=\"text\" value=\"{endpoint}\" maxlength=\"128\" placeholder=\"http://my-server:65367/api/votes\"></input></div></div><div class=\"row\"><div class=\"col-label\"> <label for=\"apikey\">API key:</label></div><div class=\"col-input\"> <input id=\"apikey\" name=\"apikey\" type=\"text\" value=\"{apikey}\" maxlength=\"32\" required=\"required\"></input></div></div><div class=\"row\"><div class=\"col-label\"> <label for=\"timeout\">Time-out in ms:</label></div><div class=\"col-input\"> <input id=\"timeout\" name=\"timeout\" type=\"number\" value=\"{timeout}\" required=\"required\"></input></div></div></fieldset> <input type=\"submit\" value=\"&#x2714; Save\"/></form><p> Local IP: {local-ip}</p></body></html>";

  configPageContent.replace("{wifi-ssid-list}", getSsids());
  configPageContent.replace("{ap-ssid}", config.accessPointSsid);
  configPageContent.replace("{ap-password}", config.accessPointPassword);
  configPageContent.replace("{wifi-ssid}", config.wifiSsid);
  configPageContent.replace("{wifi-password}", config.wifiPassword);
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

