#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <cstring>

#if __has_include("secrets.h")
#include "secrets.h"
#else
#include "secrets.example.h"
#endif

namespace th4 {
inline void beginWifi() {
  if (TH4_WIFI_SSID[0] == '\0' || strcmp(TH4_WIFI_SSID, "YOUR_WIFI_SSID") == 0) {
    Serial.println("Configure TH4/common/secrets.h before connecting to Wi-Fi.");
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(TH4_WIFI_SSID, TH4_WIFI_PASSWORD);
}

inline bool wifiConnected() {
  static bool wasConnected = false;
  const bool connected = WiFi.status() == WL_CONNECTED;
  if (connected && !wasConnected) {
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  }
  if (!connected && wasConnected) Serial.println("Wi-Fi disconnected.");
  wasConnected = connected;
  return connected;
}
}  // namespace th4
