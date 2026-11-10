#include <Arduino.h>
#include <WiFi.h>

#ifndef TH3_WIFI_ROLE_STA
#define TH3_WIFI_ROLE_STA 0
#endif

#if TH3_WIFI_ROLE_STA
#if __has_include("secrets.h")
#include "secrets.h"
#else
#define TH3_WIFI_SSID "YOUR_WIFI_SSID"
#define TH3_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif
#endif

namespace {
constexpr char AP_SSID[] = "TH3-NhomXX";
constexpr char AP_PASSWORD[] = "th3demo123";  // At least 8 characters.
constexpr uint32_t WIFI_RETRY_MS = 10000;
uint32_t lastRetryMs = 0;

#if TH3_WIFI_ROLE_STA
void connectToAccessPoint() {
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
    Serial.print("STA SSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("STA IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("STA connection timed out; will retry.");
  }
}
#endif
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);

#if TH3_WIFI_ROLE_STA
  connectToAccessPoint();
#else
  WiFi.mode(WIFI_AP);
  if (WiFi.softAP(AP_SSID, AP_PASSWORD)) {
    Serial.print("AP SSID: ");
    Serial.println(AP_SSID);
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("Scan for this SSID from a phone or laptop.");
  } else {
    Serial.println("Failed to start the SoftAP.");
  }
#endif
}

void loop() {
#if TH3_WIFI_ROLE_STA
  if (WiFi.status() != WL_CONNECTED && millis() - lastRetryMs >= WIFI_RETRY_MS) {
    lastRetryMs = millis();
    Serial.println("Retrying STA connection...");
    WiFi.reconnect();
  }
#else
  delay(1000);
#endif
}
