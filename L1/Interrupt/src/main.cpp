#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t LED_PIN = 2;

constexpr uint8_t LED_CHANNEL = 0;
constexpr uint32_t LED_FREQUENCY = 5000;
constexpr uint8_t LED_RESOLUTION = 8;

// Bài 1 không debounce: bộ đếm lưu từng cạnh FALLING ISR nhận được.
volatile uint32_t capturedButtonEvents = 0;
uint32_t handledButtonEvents = 0;
bool eventQueueWasBusy = false;

void IRAM_ATTR onButtonPressed() {
  // ISR chỉ tăng bộ đếm; không gọi Serial, delay hoặc xử lý dài ở đây.
  ++capturedButtonEvents;
}

void serviceButtonInterrupt() {
  uint32_t capturedSnapshot;
  uint32_t handledSnapshot;
  bool hasPendingEvent;

  noInterrupts();
  capturedSnapshot = capturedButtonEvents;
  hasPendingEvent = handledButtonEvents < capturedSnapshot;
  if (hasPendingEvent) {
    ++handledButtonEvents;
  }
  handledSnapshot = handledButtonEvents;
  interrupts();

  if (!hasPendingEvent) {
    if (eventQueueWasBusy) {
      Serial.print("DRAINED ISR=");
      Serial.println(handledSnapshot);
      eventQueueWasBusy = false;
    }
    return;
  }

  eventQueueWasBusy = true;
  Serial.print("handled=");
  Serial.print(handledSnapshot);
  Serial.print("\tISR=");
  Serial.print(capturedSnapshot);
  Serial.print("\tpending=");
  Serial.println(capturedSnapshot - handledSnapshot);

  ledcWrite(LED_CHANNEL, 0);
  delay(80);
  ledcWrite(LED_CHANNEL, 255);
  delay(80);
}

void fadeTo(int startDuty, int endDuty) {
  const int step = (startDuty < endDuty) ? 1 : -1;

  for (int duty = startDuty; ; duty += step) {
    ledcWrite(LED_CHANNEL, duty);
    serviceButtonInterrupt();
    delay(10);

    if (duty == endDuty) {
      break;
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    onButtonPressed,
    FALLING
  );

  Serial.println("Interrupt test ready");
}

void loop() {
  fadeTo(0, 255);
  fadeTo(255, 0);
}
