#include "time.h"
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Seconds since Jan 1, 1970 at the moment when this board was booted.
unsigned long boot_time = 0;

bool sendNTPpacket(WiFiUDP& udp, IPAddress& address)
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
  // you can send a packet requesting a timestamp.
  //NTP requests are to port 123
  if (!udp.beginPacket(address, 123)) return false;
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  return udp.endPacket();
}


unsigned long getBootTimeFromNtp() {
  unsigned int localPort = 2390;  // local port to listen for UDP packets
  IPAddress timeServerIP; // time.nist.gov NTP server address
  const char* ntpServerName = "time.nist.gov";
  const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
  byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

  WiFiUDP udp;
  udp.begin(localPort);
  WiFi.hostByName(ntpServerName, timeServerIP); 

  if (!sendNTPpacket(udp, timeServerIP)) return 0;

  int cb = udp.parsePacket();
  for (int retries = 0; !cb && (retries < 100); retries++) {
    cb = udp.parsePacket();
    if (!cb) delay(10);
  }
  if (!cb) return 0;
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
  return secsSince1900 - seventyYears - seconds_since_start;
}

void initTime() {
  if (boot_time != 0) return;
  boot_time = getBootTimeFromNtp();
}

unsigned long now() {
  return boot_time + millis() / 1000L;
}
