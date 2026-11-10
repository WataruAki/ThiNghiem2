#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <cstdio>
#include <cstring>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define TH3_WIFI_SSID "YOUR_WIFI_SSID"
#define TH3_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#define TH3_MQTT_GROUP "nhom-XX"
#define TH3_MQTT_CLIENT_ID "esp32-th3-kit-1"
#define TH3_MQTT_HOST "test.mosquitto.org"
#define TH3_MQTT_PORT 1883
#endif

namespace {
constexpr uint8_t LED_PIN = 2;
constexpr uint32_t RETRY_INTERVAL_MS = 5000;
constexpr uint32_t PUBLISH_INTERVAL_MS = 2000;
WiFiClient networkClient;
PubSubClient mqtt(networkClient);
char topicCounter[96];
char topicLed[96];
uint32_t lastWifiAttemptMs = 0;
uint32_t lastMqttAttemptMs = 0;
uint32_t lastPublishMs = 0;
uint32_t counter = 0;

void onMessage(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, topicLed) != 0 || length == 0) return;

  if (payload[0] == '1') {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("MQTT command: LED ON");
  } else if (payload[0] == '0') {
    digitalWrite(LED_PIN, LOW);
    Serial.println("MQTT command: LED OFF");
  } else {
    Serial.println("MQTT LED payload must be 1 or 0.");
  }
}

void beginWiFi() {
  lastWifiAttemptMs = millis();
  Serial.print("Connecting to SSID: ");
  Serial.println(TH3_WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(TH3_WIFI_SSID, TH3_WIFI_PASSWORD);
}

void maintainMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (mqtt.connected() || millis() - lastMqttAttemptMs < RETRY_INTERVAL_MS) return;

  lastMqttAttemptMs = millis();
  char clientId[64];
  snprintf(
    clientId,
    sizeof(clientId),
    "%s-%06lX",
    TH3_MQTT_CLIENT_ID,
    static_cast<unsigned long>(ESP.getEfuseMac() & 0xFFFFFFULL)
  );
  Serial.printf("Connecting MQTT broker %s:%u as %s...\n", TH3_MQTT_HOST, TH3_MQTT_PORT, clientId);

  if (mqtt.connect(clientId)) {
    Serial.println("MQTT connected.");
    if (mqtt.subscribe(topicLed)) {
      Serial.print("Subscribed: ");
      Serial.println(topicLed);
    } else {
      Serial.println("MQTT subscribe failed.");
    }
  } else {
    Serial.printf("MQTT connect failed, state=%d\n", mqtt.state());
  }
}

void handleSerialPublish() {
  while (Serial.available() > 0) {
    const char command = static_cast<char>(Serial.read());
    if (command != '0' && command != '1') continue;

    if (mqtt.connected()) {
      const char payload[] = {command, '\0'};
      const bool sent = mqtt.publish(topicLed, payload);
      Serial.printf("Published LED command %c to %s (%s)\n", command, topicLed, sent ? "ok" : "failed");
    } else {
      Serial.println("MQTT is disconnected; command was not sent.");
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  snprintf(topicCounter, sizeof(topicCounter), "bcse/th3/%s/count", TH3_MQTT_GROUP);
  snprintf(topicLed, sizeof(topicLed), "bcse/th3/%s/led", TH3_MQTT_GROUP);

  mqtt.setServer(TH3_MQTT_HOST, TH3_MQTT_PORT);
  mqtt.setCallback(onMessage);
  mqtt.setKeepAlive(30);
  mqtt.setBufferSize(256);
  beginWiFi();

  Serial.print("Counter topic: ");
  Serial.println(topicCounter);
  Serial.print("LED command topic: ");
  Serial.println(topicLed);
  Serial.println("Type 1 or 0 in Serial Monitor to publish an LED command.");
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    if (millis() - lastWifiAttemptMs >= RETRY_INTERVAL_MS) beginWiFi();
    delay(5);
    return;
  }

  maintainMqtt();
  if (mqtt.connected()) {
    mqtt.loop();
    handleSerialPublish();

    if (millis() - lastPublishMs >= PUBLISH_INTERVAL_MS) {
      lastPublishMs = millis();
      char payload[16];
      snprintf(payload, sizeof(payload), "%lu", static_cast<unsigned long>(counter++));
      if (mqtt.publish(topicCounter, payload)) {
        Serial.printf("Published %s -> %s\n", payload, topicCounter);
      } else {
        Serial.println("MQTT publish failed.");
      }
    }
  }
  delay(2);
}
