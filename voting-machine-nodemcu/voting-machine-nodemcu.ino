#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Pushbutton.h>
#include <ESP8266mDNS.h>
#include "time.h"
#include "server.h"
#include "config.h"

#define LED_PIN D4   // LoLin has LED at GPIO2 (D4)

#define GREEN_BUTTON_PIN D3
#define YELLOW_BUTTON_PIN D1
#define RED_BUTTON_PIN D2

#define GREEN_LED_PIN D7
#define YELLOW_LED_PIN D6
#define RED_LED_PIN D5

const uint8 VOTE_GREEN = 0;
const uint8 VOTE_YELLOW = 1;
const uint8 VOTE_RED = 2;


Configuration config;

long greenCounter = 0, yellowCounter = 0, redCounter = 0;
unsigned long totalGreen = 0, totalYellow = 0, totalRed = 0;
Pushbutton green(GREEN_BUTTON_PIN, PULL_UP_DISABLED), yellow(YELLOW_BUTTON_PIN), red(RED_BUTTON_PIN);

void ledOn();
void ledOff();
bool connectToWiFi(int waitTimeInSeconds);
bool enableAccessPoint();
bool isServer();

bool checkButtons();
void sendCounters();
void reset(bool fullReset);
void handleInput();


bool isConnected() {
  return WiFi.status() == WL_CONNECTED;
}


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  Serial.begin(115200);

  ledOn();
  digitalWrite(GREEN_LED_PIN, 1);
  delay(500);
  digitalWrite(YELLOW_LED_PIN, 1);
  delay(500);
  digitalWrite(RED_LED_PIN, 1);
  ledOff();

  WiFi.persistent(true);

  SPIFFS.begin();
  config.load();

  if (!config.autoConnectToWifi) {
    WiFi.mode(WIFI_OFF);
  }
  if (config.autoConnectToWifi && !isConnected()) {
    connectToWiFi(10);
  }
  if (isConnected()) {
    Serial.print("Local IP: ");  Serial.println(WiFi.localIP());
    initTime();
    if (!MDNS.begin("vote")) {
      Serial.println("Could not start mDNS responder - I will not be reachable by DNS name vote.local");
    }
  } else {
    Serial.println("Not connected to WiFi.");
  }

  enableAccessPoint();
  startWebServer();

  digitalWrite(GREEN_LED_PIN, 0);
  digitalWrite(YELLOW_LED_PIN, 0);
  digitalWrite(RED_LED_PIN, 0);
}


void loop() {
  handleInput();
  checkButtons();
  if (config.sendToEndpoint) {
    sendCounters();
  }
  if (isServer()) {
    processWebRequests();
  }
  // Wait a bit before scanning again
  delay(10);
}


void ledOn() {
  digitalWrite(LED_PIN, 0);
}


void ledOff() {
  digitalWrite(LED_PIN, 1);
}


bool connectToWiFi(int waitTimeInSeconds) {
  if (config.wifiSsid.length() < 1) {
    Serial.println("WiFi: not configured");
    return false;
  }
  ledOn();

  while(isConnected()) {
    WiFi.disconnect();
    delay(100);
  }

  if (WiFi.getMode() == WIFI_OFF) {
    WiFi.mode(WIFI_STA);
  } else if (WiFi.getMode() == WIFI_AP) {
    WiFi.mode(WIFI_AP_STA);
  }

  Serial.print("WiFi: '"); Serial.print(config.wifiSsid); Serial.print("'");
  WiFi.begin(config.wifiSsid.c_str(), config.wifiPassword.c_str());
  for (int i = 0; i < (waitTimeInSeconds * 10) && !isConnected(); i++) {
    delay(100);
    Serial.print('.');
  }

  ledOff();
  
  if (!isConnected()) {
    Serial.println(" failed.");
    return false;
  }
  
  Serial.println(" ok.");
  return true;
}

bool enableAccessPoint() {
  if (WiFi.getMode() == WIFI_STA) {
    WiFi.mode(WIFI_AP_STA);
  }
  if (WiFi.getMode() == WIFI_OFF) {
    WiFi.mode(WIFI_AP);
  }

  Serial.print("AP: ");
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  bool result = WiFi.softAP(config.accessPointSsid.c_str(), config.accessPointPassword.c_str());
  if (!result) {
    Serial.println("failed.");
    return false;
  }
  Serial.println(WiFi.softAPIP());
  return true;
}


bool isServer() {
  return WiFi.getMode() == WIFI_AP || WiFi.getMode() == WIFI_AP_STA;
}


void waitForRelease(Pushbutton& button) {
  while(button.isPressed()) {
    delay(10);
  }
}


bool checkButtons() {
  bool changed = false;
  if (green.getSingleDebouncedPress()) {
    greenCounter++;
    totalGreen++;
    changed = true;
    digitalWrite(GREEN_LED_PIN, 1);
    waitForRelease(green);
    digitalWrite(GREEN_LED_PIN, 0);
  }
  if (yellow.getSingleDebouncedPress()) {
    yellowCounter++;
    totalYellow++;
    changed = true;
    digitalWrite(YELLOW_LED_PIN, 1);
    waitForRelease(yellow);
    digitalWrite(YELLOW_LED_PIN, 0);
  }
  if (red.getSingleDebouncedPress()) {
    redCounter++;
    totalRed++;
    changed = true;
    digitalWrite(RED_LED_PIN, 1);
    waitForRelease(red);
    digitalWrite(RED_LED_PIN, 0);
  }
  return changed;
}


void reset(bool fullReset) {
  redCounter = 0;
  greenCounter = 0;
  yellowCounter = 0;
  if (fullReset) {
    totalGreen = 0;
    totalYellow = 0;
    totalRed = 0;
  }
}


void flashButtons() {
  digitalWrite(GREEN_LED_PIN, 1);
  digitalWrite(YELLOW_LED_PIN, 1);
  digitalWrite(RED_LED_PIN, 1);
  delay(500);
  digitalWrite(GREEN_LED_PIN, 0);
  digitalWrite(YELLOW_LED_PIN, 0);
  digitalWrite(RED_LED_PIN, 0);
}


bool sendVote(HTTPClient& client, uint8 color) {
  String data = String("{\"value\": ") + color + "}";
  int statusCode = client.POST(data);
  return (statusCode >= 200 && statusCode < 300);
}


unsigned long lastConnectAttempt = 0;

void sendCounters() {
  if (!greenCounter && !yellowCounter && !redCounter) return;

  if (!isConnected()) {
    if (now() - lastConnectAttempt > 10) {
      lastConnectAttempt = now();
      connectToWiFi(1);
      if (!isConnected()) return;
      initTime();
    } else {
      return;
    }
  }

  HTTPClient client;
  client.setTimeout(config.timeoutInMillis);
  client.addHeader("Authorization", String("Bearer ") + config.apiKey);
  if (!client.begin(config.targetEndpoint)) {
    Serial.print("Failed to connect to endpoint.");
    return;
  }

  if (greenCounter > 0) {
    if (sendVote(client, VOTE_GREEN)) {
      greenCounter--;
      Serial.println("green");
    } else {
      Serial.println("!green");
    }
  }
  if (yellowCounter > 0) {
    if (sendVote(client, VOTE_YELLOW)) {
      yellowCounter--;
      Serial.println("yellow");
    } else {
      Serial.println("!yellow");
    }
  }
  if (redCounter > 0) {
    if (sendVote(client, VOTE_RED)) {
      redCounter--;
      Serial.println("red");
    } else {
      Serial.println("!red");
    }
  }
  client.end();
}


void handleInput() {
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    switch (input) {
      case 'a':
        enableAccessPoint();
        break;
      case 'c':
        Serial.print("green: "); Serial.println(greenCounter);
        Serial.print("yellow: "); Serial.println(yellowCounter);
        Serial.print("red: "); Serial.println(redCounter);
        break;
      case 'd':
        WiFi.printDiag(Serial);
        Serial.setDebugOutput(true);
        break;
      case 'D':
        Serial.setDebugOutput(false);
        WiFi.printDiag(Serial);
        break;
      case 'A':
        WiFi.softAPdisconnect(true);
        break;
      case 'r':
        flashButtons();
        delay(250);
        flashButtons();
        delay(250);
        flashButtons();
        reset(false);
        break;
      case 'R':
        flashButtons();
        delay(250);
        flashButtons();
        delay(250);
        flashButtons();
        reset(true);
        break;
      case 's':
        config.printTo(Serial);
        Serial.println();
        break;
      case 't':
        Serial.println(now());
        break;
      case 'w':
        connectToWiFi(5);
        break;
      case 'W':
        WiFi.disconnect();
        break;
      default:
        // ignore
        break;
    }

    // read the rest of serial input and discard.
    while(Serial.available() > 0) {
      Serial.read();
    }
  }
}

