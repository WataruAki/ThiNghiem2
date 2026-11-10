#include <WiFi.h>

#include "../../common/MqttSession.h"

namespace {
th4::MqttSession session;
uint32_t lastPublish = 0;
}  // namespace

void setup() {
  Serial.begin(115200);
  th4::beginWifi();
  session.begin("th4-b2");
}

void loop() {
  const bool connected = session.poll(th4::wifiConnected());
  const uint32_t now = millis();
  if (connected && now - lastPublish >= th4::SAMPLE_INTERVAL_MS) {
    lastPublish = now;
    JsonDocument doc;
    th4::fillSensorPayload(doc, th4::demoReading());
    session.publish(doc);
  }
  delay(2);
}
