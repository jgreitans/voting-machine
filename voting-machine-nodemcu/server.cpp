#include "server.h"
#include <ESP8266WiFi.h>

WiFiServer server(80);

void sendResponse(WiFiClient& client, String status, String& data) {
  client.print("HTTP/1.1 "); client.print(status); client.print("\r\n");
  client.print("Connection: close\r\n");
  client.print("Content-type: text/html\r\n");
  if (data.length() > 0) {
    client.print("Content-length: "); client.print(data.length()); client.print("\r\n");
    client.print("\r\n");
    client.print(data);
  } else {
    client.print("Content-length: 0\r\n");
    client.print("\r\n");
  }
}

void servePages() {
  WiFiClient client = server.available();
  if (!client) return;
  String data;
  while (client.connected()) {
    if (client.available()) {
      String line = client.readStringUntil('\r');
      data.concat(line);
      data.concat('\r');
      Serial.print(line);
      if (line.length() == 1 && line[0] == '\n')  {
        sendResponse(client, "200 OK", "<html><body><pre>" + data + "</pre></body></html>");
        break;
      }
    }
  }

  delay(1);
  client.stop();
}
