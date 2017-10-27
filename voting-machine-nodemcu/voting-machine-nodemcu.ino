#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "wifi-password.h"

#define LED_PIN 2   // LoLin has LED at GPIO2 (D0)

// Seconds since Jan 1, 1970 at the moment when this board was booted.
unsigned long boot_time = 0;

void initLed();
void initTime();
void ledOn();
void ledOff();
bool ensureConnection();
bool isConnected();
unsigned long sendNTPpacket(WiFiUDP& udp, IPAddress& address);


void setup() {
  Serial.begin(115200);
  initLed();
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
}

void loop() {
  ensureConnection();
  // Wait a bit before scanning again
  delay(1000);
}



void initLed() {
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
