#include <WebServer.h>
#include <WiFi.h>

#include "../../common/SensorPayload.h"

namespace {
WebServer server(80);
bool serverStarted = false;

void handleSensor() {
  JsonDocument doc;
  th4::fillSensorPayload(doc, th4::demoReading());
  String body;
  serializeJson(doc, body);
  server.sendHeader("Cache-Control", "no-store");
  server.send(200, "application/json", body);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  server.on("/api/sensor", HTTP_GET, handleSensor);
  th4::beginWifi();
  Serial.println("REST route: GET /api/sensor (demo sensor)");
}

void loop() {
  if (th4::wifiConnected()) {
    if (!serverStarted) {
      server.begin();
      serverStarted = true;
    }
    server.handleClient();
  }
  delay(2);
}
