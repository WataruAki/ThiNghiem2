#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#define TH3_WIFI_SSID "YOUR_WIFI_SSID"
#define TH3_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

namespace {
constexpr uint8_t LED_PIN = 2;
constexpr uint32_t WIFI_RETRY_MS = 10000;
WebServer server(80);
bool ledOn = false;
uint32_t lastWifiAttemptMs = 0;

void setLed(bool state) {
  ledOn = state;
  digitalWrite(LED_PIN, ledOn ? HIGH : LOW);
}

void sendHomePage() {
  String page;
  page.reserve(1100);
  page += F("<!doctype html><html><head><meta name='viewport' content='width=device-width,initial-scale=1'>");
  page += F("<meta charset='utf-8'><title>ESP32 LED</title><style>");
  page += F("body{font:18px sans-serif;max-width:520px;margin:40px auto;padding:0 16px;text-align:center}");
  page += F("a{display:inline-block;margin:8px;padding:14px 24px;background:#1769aa;color:white;text-decoration:none;border-radius:8px}");
  page += F(".state{font-size:24px;font-weight:bold}</style></head><body><h1>ESP32 LED Control</h1><p class='state'>LED is ");
  page += ledOn ? F("ON") : F("OFF");
  page += F("</p><a href='/on'>ON</a><a href='/off'>OFF</a><a href='/toggle'>TOGGLE</a></body></html>");
  server.send(200, "text/html; charset=utf-8", page);
}

void redirectHome() {
  server.sendHeader("Location", "/", true);
  server.send(303, "text/plain", "LED state updated");
}

void beginWiFi() {
  lastWifiAttemptMs = millis();
  Serial.print("Connecting to SSID: ");
  Serial.println(TH3_WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(TH3_WIFI_SSID, TH3_WIFI_PASSWORD);

  const uint32_t startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 20000) {
    delay(500);
    Serial.print('.');
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("HTTP server IP: http://");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi not connected yet; retrying in loop.");
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  setLed(false);
  beginWiFi();

  server.on("/", HTTP_GET, sendHomePage);
  server.on("/on", HTTP_GET, []() { setLed(true); redirectHome(); });
  server.on("/off", HTTP_GET, []() { setLed(false); redirectHome(); });
  server.on("/toggle", HTTP_GET, []() { setLed(!ledOn); redirectHome(); });
  server.onNotFound([]() { server.send(404, "text/plain", "Not found"); });
  server.begin();
  Serial.println("HTTP server started on port 80.");
}

void loop() {
  server.handleClient();
  if (WiFi.status() != WL_CONNECTED && millis() - lastWifiAttemptMs >= WIFI_RETRY_MS) {
    beginWiFi();
  }
  delay(2);
}
