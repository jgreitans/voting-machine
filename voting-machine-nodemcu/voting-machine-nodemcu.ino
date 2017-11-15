#include <ESP8266WiFi.h>
#include <Pushbutton.h>
#include "wifi-password.h"
#include "time.h"

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


const char* targetHost = "192.168.1.100";
const int port = 65367;
const char* apiKey = "3db17c25b53248a2be53adafd98763b6";

long greenCounter = 0, yellowCounter = 0, redCounter = 0;
Pushbutton green(GREEN_BUTTON_PIN, PULL_UP_DISABLED), yellow(YELLOW_BUTTON_PIN), red(RED_BUTTON_PIN);
bool canSendData = false;

void ledOn();
void ledOff();
bool ensureConnection();
bool isConnected();
bool checkButtons();
void sendCounters();
void reset();
void handleInput();
bool enableAccessPoint();


void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  Serial.begin(115200);

  digitalWrite(GREEN_LED_PIN, 1);
  delay(500);
  digitalWrite(YELLOW_LED_PIN, 1);
  delay(500);
  digitalWrite(RED_LED_PIN, 1);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  while(isConnected()) {
    delay(100);
  }

  canSendData = ensureConnection();
  if (!canSendData) enableAccessPoint();

  digitalWrite(GREEN_LED_PIN, 0);
  digitalWrite(YELLOW_LED_PIN, 0);
  digitalWrite(RED_LED_PIN, 0);
}

void loop() {
  handleInput();
  checkButtons();
  sendCounters();
  // Wait a bit before scanning again
  delay(10);
}



void ledOn() {
  digitalWrite(LED_PIN, 0);
}

void ledOff() {
  digitalWrite(LED_PIN, 1);
}

bool ensureConnection() {
  if (isConnected()) {
    return true;
  }
  WiFi.mode(WIFI_STA);
  ledOn();
  Serial.println();
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  for (int i = 0; i < 100 && !isConnected(); i++) {
    delay(100);
    Serial.print('.');
  }
  if (!isConnected()) {
    Serial.println(" FAILED.");
    ledOff();
    return false;
  }
  Serial.print(' ');
  Serial.println(WiFi.localIP());
  ledOff();

  initTime();

  return true;
}

bool isConnected() {
  return WiFi.status() == WL_CONNECTED;
}

bool checkButtons() {
  bool changed = false;
  if (green.getSingleDebouncedPress()) {
    greenCounter++;
    changed = true;
    digitalWrite(GREEN_LED_PIN, 1);
  }
  if (yellow.getSingleDebouncedPress()) {
    yellowCounter++;
    changed = true;
    digitalWrite(YELLOW_LED_PIN, 1);
  }
  if (red.getSingleDebouncedPress()) {
    redCounter++;
    changed = true;
    digitalWrite(RED_LED_PIN, 1);
  }

  if (green.getSingleDebouncedRelease()) digitalWrite(GREEN_LED_PIN, 0);
  if (yellow.getSingleDebouncedRelease()) digitalWrite(YELLOW_LED_PIN, 0);
  if (red.getSingleDebouncedRelease()) digitalWrite(RED_LED_PIN, 0);

  return changed;
}

void reset() {
  redCounter = 0;
  greenCounter = 0;
  yellowCounter = 0;
}

void handleInput() {
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    switch (input) {
      case 'r':
        flashButtons();
        delay(500);
        flashButtons();
        delay(500);
        flashButtons();
        reset();
        break;
      case 'a':
        enableAccessPoint();
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

void flashButtons() {
  digitalWrite(GREEN_LED_PIN, 1);
  digitalWrite(YELLOW_LED_PIN, 1);
  digitalWrite(RED_LED_PIN, 1);
  delay(500);
  digitalWrite(GREEN_LED_PIN, 0);
  digitalWrite(YELLOW_LED_PIN, 0);
  digitalWrite(RED_LED_PIN, 0);
}


bool sendVote(WiFiClient& client, const char* targetHost, uint8 color) {
  String data = String("{\"value\": ") + color + "}";
  client.print(String("POST /api/votes HTTP/1.1\r\n") +
      "Host: " + targetHost + "\r\n" +
      "Connection: close\r\n" +
      "Authorization: Bearer " + apiKey + "\r\n" +
      "Content-type: application/json\r\n" + 
      "Content-length: " + data.length() + "\r\n" +
      "\r\n" +
      data
  );

  if (client.connected())
  {
    if (client.available())
    {
      String line = client.readStringUntil('\n');
      int spacePos = line.indexOf(' ');
      if (spacePos > 0 && spacePos <= (line.length() - 3)) {
        // HTTP status code is one of 2xx
        if (line[spacePos + 1] == '2') {
          return true;
        } else {
          Serial.println(line);
        }
      }
      return false;
    }
  }
  return false;
}

void sendCounters() {
  if (!greenCounter && !yellowCounter && !redCounter) return;

  if (!canSendData) return;

  WiFiClient client;
  if (client.connect(targetHost, port)) {
    if (greenCounter > 0) {
      if (sendVote(client, targetHost, VOTE_GREEN)) {
        greenCounter--;
        Serial.println("green");
      }
    }
    if (yellowCounter > 0) {
      if (sendVote(client, targetHost, VOTE_YELLOW)) {
        yellowCounter--;
        Serial.println("yellow");
      }
    }
    if (redCounter > 0) {
      if (sendVote(client, targetHost, VOTE_RED)) {
        redCounter--;
        Serial.println("red");
      }
    }
  } else {
    Serial.println("Could not connect to target host.");
    delay(100);
  }
  client.stop();
}

bool enableAccessPoint() {
  if (isConnected()) {
    WiFi.disconnect();
  }
  WiFi.mode(WIFI_AP);

  Serial.print("Setting up access point... ");
  WiFi.softAPConfig(IPAddress(192, 168, 1, 1), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));
  bool result = WiFi.softAP(accessPointSsid, accessPointPassword);
  if (!result) {
    Serial.println("failed.");
    return false;
  }
  Serial.println("ok.");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());
  return true;
}

