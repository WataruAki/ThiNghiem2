#pragma once

#include <PubSubClient.h>
#include <cstdio>

#include "SensorPayload.h"

namespace th4 {
class MqttSession {
 public:
  void begin(const char* projectName, bool receiveLedCommands = false) {
    receiveLedCommands_ = receiveLedCommands;
    if (!validStudentId()) {
      Serial.println("Set TH4_SV_ID in TH4/common/secrets.h to your own ID (1-48 letters, digits, _ or -).");
      return;
    }
    snprintf(telemetryTopic_, sizeof(telemetryTopic_), "aet2022/%s/telemetry", TH4_SV_ID);
    snprintf(ledTopic_, sizeof(ledTopic_), "aet2022/%s/cmd/led", TH4_SV_ID);
    snprintf(clientId_, sizeof(clientId_), "%s-%012llX", projectName,
             static_cast<unsigned long long>(ESP.getEfuseMac() & 0xFFFFFFFFFFFFULL));
    tcp_.setTimeout(2);
    mqtt_.setServer(TH4_MQTT_HOST, TH4_MQTT_PORT);
    mqtt_.setSocketTimeout(2);
    ready_ = mqtt_.setBufferSize(512);
    if (!ready_) {
      Serial.println("Could not allocate the MQTT packet buffer.");
      return;
    }
    Serial.printf("MQTT broker: %s:%u\n", TH4_MQTT_HOST, static_cast<unsigned>(TH4_MQTT_PORT));
    Serial.printf("Telemetry: %s\n", telemetryTopic_);
    if (receiveLedCommands_) Serial.printf("LED commands: %s\n", ledTopic_);
  }

  PubSubClient& client() { return mqtt_; }
  const char* ledTopic() const { return ledTopic_; }

  bool poll(bool wifiUp) {
    if (!ready_) return false;
    if (!wifiUp) {
      if (attempted_) tcp_.stop();
      attempted_ = false;
      return false;
    }
    if (mqtt_.connected()) return mqtt_.loop();

    const uint32_t now = millis();
    if (attempted_ && now - lastAttempt_ < 5000) return false;
    attempted_ = true;
    lastAttempt_ = now;

    const bool connected = TH4_MQTT_USER[0] == '\0'
        ? mqtt_.connect(clientId_)
        : mqtt_.connect(clientId_, TH4_MQTT_USER, TH4_MQTT_PASSWORD);
    if (!connected) {
      Serial.printf("MQTT connect failed (%d).\n", mqtt_.state());
      return false;
    }
    if (receiveLedCommands_ && !mqtt_.subscribe(ledTopic_, 1)) {
      Serial.println("Could not send LED subscription. Retrying connection.");
      mqtt_.disconnect();
      return false;
    }
    Serial.printf("MQTT connected as %s.\n", clientId_);
    return true;
  }

  bool publish(const JsonDocument& doc) {
    char payload[256];
    if (measureJson(doc) >= sizeof(payload)) {
      Serial.println("Telemetry exceeds the payload buffer.");
      return false;
    }
    serializeJson(doc, payload);
    if (!mqtt_.publish(telemetryTopic_, payload)) {
      Serial.println("MQTT publish failed.");
      return false;
    }
    Serial.printf("%s %s\n", telemetryTopic_, payload);
    return true;
  }

 private:
  static bool validStudentId() {
    const size_t length = strlen(TH4_SV_ID);
    if (length == 0 || length > 48 || strcmp(TH4_SV_ID, "YOUR_SV_ID") == 0) return false;
    for (size_t i = 0; i < length; ++i) {
      const char c = TH4_SV_ID[i];
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') || c == '_' || c == '-')) return false;
    }
    return true;
  }

  WiFiClient tcp_;
  PubSubClient mqtt_{tcp_};
  char telemetryTopic_[80]{};
  char ledTopic_[80]{};
  char clientId_[24]{};
  bool ready_ = false;
  bool receiveLedCommands_ = false;
  bool attempted_ = false;
  uint32_t lastAttempt_ = 0;
};
}  // namespace th4
