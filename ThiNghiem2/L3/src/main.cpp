#include <Arduino.h>

#define ADC_PIN 34                 // ADC1_CH6; đổi sang GPIO27 cho thử ADC2.
#define ENABLE_WIFI_FOR_ADC2_TEST 0

#if ENABLE_WIFI_FOR_ADC2_TEST
#include <WiFi.h>
#endif

constexpr uint8_t LED_PIN = 18;
constexpr uint8_t LED_CHANNEL = 0;
constexpr uint32_t LED_FREQUENCY = 5000;
constexpr uint8_t LED_RESOLUTION = 8;
constexpr size_t FILTER_WINDOW = 8;

int samples[FILTER_WINDOW] = {};
size_t sampleIndex = 0;
bool filterPrimed = false;

int movingAverage(int input) {
  if (!filterPrimed) {
    for (size_t i = 0; i < FILTER_WINDOW; ++i) {
      samples[i] = input;
    }
    filterPrimed = true;
  }

  samples[sampleIndex] = input;
  sampleIndex = (sampleIndex + 1) % FILTER_WINDOW;

  long sum = 0;
  for (size_t i = 0; i < FILTER_WINDOW; ++i) {
    sum += samples[i];
  }
  return static_cast<int>(sum / FILTER_WINDOW);
}

void setup() {
  Serial.begin(115200);
  analogReadResolution(12);
  analogSetPinAttenuation(ADC_PIN, ADC_11db);

  ledcSetup(LED_CHANNEL, LED_FREQUENCY, LED_RESOLUTION);
  ledcAttachPin(LED_PIN, LED_CHANNEL);

#if ENABLE_WIFI_FOR_ADC2_TEST
  WiFi.mode(WIFI_STA);
#endif

  Serial.println(">tho:0\t>loc:0");
}

void loop() {
  const int raw = analogRead(ADC_PIN);
  const int filtered = movingAverage(raw);
  const int duty = map(filtered, 0, 4095, 0, 255);

  ledcWrite(LED_CHANNEL, duty);

  Serial.print(">tho:");
  Serial.print(raw);
  Serial.print('\t');
  Serial.print(">loc:");
  Serial.print(filtered);
  Serial.println();

  delay(20);
}
