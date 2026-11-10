#include <Adafruit_SHT31.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <Wire.h>
#include <cmath>

#include "../../common/MqttSession.h"

namespace {
constexpr uint8_t LED_PIN = 14;
constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t SDA_PIN = 21;
constexpr uint8_t SCL_PIN = 22;
constexpr uint8_t SHT31_ADDRESS = 0x44;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint32_t DEBOUNCE_MS = 35;

Adafruit_SHT31 sht31;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
th4::MqttSession session;
th4::Reading reading{0, 0, false, "sht31"};
bool sensorReady = false;
bool oledReady = false;
bool rawPressed = false;
bool pressed = false;
bool ledOn = false;
bool stateChanged = true;
bool redraw = true;
bool mqttWasConnected = false;
uint32_t pressCount = 0;
uint32_t rawChangedAt = 0;
uint32_t lastSample = 0;
uint32_t lastPublish = 0;

void setLed(bool on) {
  ledOn = on;
  digitalWrite(LED_PIN, on ? HIGH : LOW);
  stateChanged = true;
  redraw = true;
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, session.ledTopic()) != 0 || length != 1) return;
  if (payload[0] != '1' && payload[0] != '0') return;
  setLed(payload[0] == '1');
  Serial.printf("Dashboard LED command: %c\n", payload[0]);
}

void readButton(uint32_t now) {
  const bool down = digitalRead(BUTTON_PIN) == LOW;
  if (down != rawPressed) {
    rawPressed = down;
    rawChangedAt = now;
  }
  if (rawPressed == pressed || now - rawChangedAt < DEBOUNCE_MS) return;

  pressed = rawPressed;
  stateChanged = true;
  redraw = true;
  if (pressed) {
    ++pressCount;
    setLed(!ledOn);
  }
}

void sampleSensor() {
  reading.valid = sensorReady && sht31.readBoth(&reading.temp, &reading.hum) &&
                  std::isfinite(reading.temp) && std::isfinite(reading.hum);
  redraw = true;
}

void drawDisplay(bool connected) {
  if (!oledReady || !redraw) return;
  redraw = false;
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println("TH4 | ESP32 MQTT");
  if (reading.valid) {
    oled.print("Temp: ");
    oled.print(reading.temp, 1);
    oled.println(" C");
    oled.print("Hum : ");
    oled.print(reading.hum, 1);
    oled.println(" %");
  } else {
    oled.println("SHT31: no reading");
    oled.println();
  }
  oled.print("LED : ");
  oled.println(ledOn ? "ON" : "OFF");
  oled.print(pressed ? "BTN : DOWN  #" : "BTN : UP    #");
  oled.println(pressCount);
  oled.println(connected ? "MQTT: ONLINE" : "MQTT: OFFLINE");
  oled.display();
}
}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  setLed(false);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  rawPressed = digitalRead(BUTTON_PIN) == LOW;
  pressed = rawPressed;

  Wire.begin(SDA_PIN, SCL_PIN);
  sensorReady = sht31.begin(SHT31_ADDRESS);
  Wire.beginTransmission(OLED_ADDRESS);
  oledReady = Wire.endTransmission() == 0 &&
             oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS, true, false);
  if (!sensorReady) Serial.println("SHT31 not detected. Telemetry temp/hum will be null.");
  if (!oledReady) Serial.println("SSD1306 not detected. Check wiring and address.");
  sampleSensor();
  drawDisplay(false);

  th4::beginWifi();
  session.begin("th4-b3", true);
  session.client().setCallback(onMqttMessage);
}

void loop() {
  readButton(millis());
  const bool connected = session.poll(th4::wifiConnected());
  const uint32_t now = millis();
  if (now - lastSample >= th4::SAMPLE_INTERVAL_MS) {
    lastSample = now;
    sampleSensor();
  }
  const bool justConnected = connected && !mqttWasConnected;
  if (connected != mqttWasConnected) redraw = true;
  mqttWasConnected = connected;

  if (connected && (justConnected || stateChanged ||
                    now - lastPublish >= th4::SAMPLE_INTERVAL_MS)) {
    lastPublish = now;
    stateChanged = false;
    JsonDocument doc;
    th4::fillSensorPayload(doc, reading);
    doc["button"] = pressed;
    doc["presses"] = pressCount;
    doc["led"] = ledOn;
    session.publish(doc);
  }
  drawDisplay(connected);
  delay(2);
}
