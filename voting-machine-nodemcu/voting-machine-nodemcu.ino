#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Pushbutton.h>
#include "wifi-password.h"

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

// Seconds since Jan 1, 1970 at the moment when this board was booted.
unsigned long boot_time = 0;
long greenCounter = 0, yellowCounter = 0, redCounter = 0;
Pushbutton green(GREEN_BUTTON_PIN, PULL_UP_DISABLED), yellow(YELLOW_BUTTON_PIN), red(RED_BUTTON_PIN);

void initTime();
void ledOn();
void ledOff();
bool ensureConnection();
bool isConnected();
unsigned long sendNTPpacket(WiFiUDP& udp, IPAddress& address);
bool checkButtons();
void sendCounters();
void reset();
void handleInput();


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
  while (!ensureConnection()) {
    delay(1000);
  }

  initTime();

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

void initTime() {
  unsigned int localPort = 2390;  // local port to listen for UDP packets
  IPAddress timeServerIP; // time.nist.gov NTP server address
  const char* ntpServerName = "time.nist.gov";
  const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

  WiFiUDP udp;
  udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(udp, timeServerIP);
  int cb = udp.parsePacket();
  while(!cb) {
    cb = udp.parsePacket();
  }
  unsigned long seconds_since_start = millis() / 1000L;
  udp.read(packetBuffer, NTP_PACKET_SIZE);

  //the timestamp starts at byte 40 of the received packet and is four bytes, or two words, long.
  // First, esxtract the two words:
  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years and time since start-up:
  boot_time = secsSince1900 - seventyYears - seconds_since_start;
  Serial.print("Unix time at boot time = ");
  Serial.println(boot_time);
}

unsigned long sendNTPpacket(WiFiUDP& udp, IPAddress& address)
{
  const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
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

  ensureConnection();

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

