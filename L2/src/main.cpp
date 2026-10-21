#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t LED_PIN = 2;
constexpr uint32_t DEBOUNCE_MS = 50;  // Thử 5, 50 và 500 ms.

volatile uint32_t rawEdgeCount = 0;
volatile uint32_t debouncedCount = 0;
volatile uint32_t lastInterruptTime = 0;

void IRAM_ATTR handleButtonInterrupt() {
  ++rawEdgeCount;

  const uint32_t now = millis();
  if (static_cast<uint32_t>(now - lastInterruptTime) >= DEBOUNCE_MS) {
    ++debouncedCount;
    lastInterruptTime = now;
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Cho phép lần nhấn đầu tiên được tính ngay sau khi khởi động.
  lastInterruptTime = millis() - DEBOUNCE_MS;
  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    handleButtonInterrupt,
    FALLING
  );

  Serial.println("raw_edges\taccepted");
}

void loop() {
  static uint32_t lastPrintTime = 0;
  const uint32_t now = millis();

  if (static_cast<uint32_t>(now - lastPrintTime) >= 250) {
    lastPrintTime = now;

    uint32_t rawSnapshot;
    uint32_t acceptedSnapshot;
    noInterrupts();
    rawSnapshot = rawEdgeCount;
    acceptedSnapshot = debouncedCount;
    interrupts();

    digitalWrite(LED_PIN, (acceptedSnapshot & 1U) ? HIGH : LOW);
    Serial.print(rawSnapshot);
    Serial.print('\t');
    Serial.println(acceptedSnapshot);
  }
}
