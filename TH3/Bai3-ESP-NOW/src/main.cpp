#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include <cstring>

#ifndef TH3_ESPNOW_ROLE_TX
#define TH3_ESPNOW_ROLE_TX 0
#endif

namespace {
struct __attribute__((packed)) Packet {
  uint32_t sequence;
  uint32_t uptimeMs;
};

#if TH3_ESPNOW_ROLE_TX
// Replace these bytes with the receiver's WiFi.macAddress() before uploading.
const uint8_t RECEIVER_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
constexpr uint32_t SEND_INTERVAL_MS = 2000;
uint32_t sequenceNumber = 0;
uint32_t lastSendMs = 0;
bool transmitterReady = false;

bool receiverMacConfigured() {
  for (uint8_t byte : RECEIVER_MAC) {
    if (byte != 0) return true;
  }
  return false;
}

void onDataSent(const uint8_t* mac, esp_now_send_status_t status) {
  (void)mac;
  Serial.print("Send status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "success" : "failed");
}
#else
void onDataReceived(const uint8_t* senderMac, const uint8_t* data, int length) {
  if (length != static_cast<int>(sizeof(Packet))) {
    Serial.printf("Ignored packet with length %d\n", length);
    return;
  }

  Packet packet{};
  memcpy(&packet, data, sizeof(packet));
  Serial.printf(
    "RX from %02X:%02X:%02X:%02X:%02X:%02X | seq=%lu | uptime=%lu ms\n",
    senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5],
    static_cast<unsigned long>(packet.sequence),
    static_cast<unsigned long>(packet.uptimeMs)
  );
}
#endif
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  const String ownMac = WiFi.macAddress();
  Serial.print("This kit MAC: ");
  Serial.println(ownMac);
  Serial.printf("Wi-Fi channel: %u\n", WiFi.channel());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW initialization failed.");
    return;
  }

#if TH3_ESPNOW_ROLE_TX
  esp_now_register_send_cb(onDataSent);
  if (!receiverMacConfigured()) {
    Serial.println("Set RECEIVER_MAC to the Receiver MAC before sending.");
    return;
  }

  esp_now_peer_info_t peer{};
  memcpy(peer.peer_addr, RECEIVER_MAC, sizeof(RECEIVER_MAC));
  peer.channel = 0;  // Use the current Wi-Fi channel; neither kit joins an AP.
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  const esp_err_t result = esp_now_add_peer(&peer);
  if (result != ESP_OK) {
    Serial.printf("Adding Receiver peer failed: %d\n", static_cast<int>(result));
    return;
  }
  transmitterReady = true;
  Serial.println("Transmitter ready.");
#else
  esp_now_register_recv_cb(onDataReceived);
  Serial.println("Receiver ready; give this kit MAC to the Transmitter.");
#endif
}

void loop() {
#if TH3_ESPNOW_ROLE_TX
  if (transmitterReady && millis() - lastSendMs >= SEND_INTERVAL_MS) {
    lastSendMs = millis();
    const Packet packet{++sequenceNumber, millis()};
    const esp_err_t result = esp_now_send(
      RECEIVER_MAC,
      reinterpret_cast<const uint8_t*>(&packet),
      sizeof(packet)
    );
    if (result != ESP_OK) {
      Serial.printf("esp_now_send failed: %d\n", static_cast<int>(result));
    }
  }
#else
  delay(10);
#endif
}
