#include "ESP8266WiFi.h"
#include "wifi-password.h"

#define LED_PIN 2   // LoLin has LED at GPIO2 (D0)

void ledInit() {
  pinMode(LED_PIN, OUTPUT);
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
  return true;
}

bool isConnected() {
  return WiFi.status() == WL_CONNECTED;
}


void setup() {
  Serial.begin(115200);
  ledInit();
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(2000);

  ensureConnection();
}

void loop() {
  ensureConnection();
  // Wait a bit before scanning again
  delay(1000);
}
