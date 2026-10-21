#include <Arduino.h>

constexpr uint8_t BUTTON_PIN = 4;
constexpr uint8_t LED_PIN = 2;

constexpr uint8_t LED_CHANNEL = 0;
constexpr uint32_t LED_FREQUENCY = 5000;
constexpr uint8_t LED_RESOLUTION = 8;

bool previousButtonState = HIGH;

void checkButtonPolling() {
  const bool currentButtonState = digitalRead(BUTTON_PIN);

  // INPUT_PULLUP: nút được nhấn sẽ đọc LOW.
  if (previousButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("Button pressed - polling");
  }

  previousButtonState = currentButtonState;
}

void fadeTo(int startDuty, int endDuty) {
  const int step = (startDuty < endDuty) ? 1 : -1;

  for (int duty = startDuty; ; duty += step) {
    ledcWrite(LED_CHANNEL, duty);
    checkButtonPolling();
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

  Serial.println("Polling test ready");
}

void loop() {
  fadeTo(0, 255);
  fadeTo(255, 0);
}
