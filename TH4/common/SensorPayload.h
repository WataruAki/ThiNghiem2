#pragma once

#include <ArduinoJson.h>
#include <esp_timer.h>

#include "WifiStation.h"

namespace th4 {
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000;

struct Reading {
  float temp;
  float hum;
  bool valid;
  const char* source;
};

inline Reading demoReading() {
  const auto step = static_cast<unsigned>((esp_timer_get_time() / 2000000) % 12);
  return {26.0f + step * 0.25f, 55.0f + step * 0.5f, true, "demo"};
}

inline void fillSensorPayload(JsonDocument& doc, const Reading& reading) {
  if (reading.valid) {
    doc["temp"] = reading.temp;
    doc["hum"] = reading.hum;
  } else {
    doc["temp"] = nullptr;
    doc["hum"] = nullptr;
  }
  doc["uptime"] = esp_timer_get_time() / 1000000;
  doc["rssi"] = WiFi.RSSI();
  doc["source"] = reading.source;
}
}  // namespace th4
