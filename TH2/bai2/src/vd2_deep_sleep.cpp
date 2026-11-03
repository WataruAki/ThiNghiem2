#include <Arduino.h>
#include <esp_sleep.h>
#include <driver/rtc_io.h>

#ifndef ENABLE_EXT0
#define ENABLE_EXT0 0
#endif

constexpr uint8_t LED_PIN = 2;
constexpr gpio_num_t BUTTON_PIN = GPIO_NUM_4;
constexpr uint64_t SLEEP_US = 10ULL * 1000000ULL;
constexpr uint32_t AWAKE_MS = 5000;

RTC_DATA_ATTR uint32_t rtcWakeCount = 0;  // Giu qua deep sleep; khong ben vung khi mat nguon.
uint32_t normalWakeCount = 0;              // RAM thuong: khoi tao lai sau deep sleep.
uint32_t bootMs = 0;
uint32_t lastLedMs = 0;
bool ledOn = false;
bool sleepFailed = false;
bool releaseMessagePrinted = false;

const char* wakeName(esp_sleep_wakeup_cause_t cause) {
    switch (cause) {
        case ESP_SLEEP_WAKEUP_TIMER: return "TIMER";
        case ESP_SLEEP_WAKEUP_EXT0: return "EXT0 (nut GPIO4)";
        case ESP_SLEEP_WAKEUP_UNDEFINED: return "UNDEFINED (cap nguon/reset, khong phai sleep wake)";
        default: return "OTHER";
    }
}

bool checkEsp(esp_err_t error, const char* operation) {
    if (error == ESP_OK) return true;
    Serial.printf("ERROR %s: %s; khong vao deep sleep\n", operation, esp_err_to_name(error));
    sleepFailed = true;
    return false;
}

void enterDeepSleep() {
    if (!checkEsp(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL), "clear wakeup")) return;
    if (!checkEsp(esp_sleep_enable_timer_wakeup(SLEEP_US), "timer wakeup")) return;
#if ENABLE_EXT0
    // Nut noi GPIO4-GND, them dien tro 10k GPIO4-3V3. Nha nut truoc khi ngu.
    if (!checkEsp(rtc_gpio_pullup_en(BUTTON_PIN), "RTC pull-up")) return;
    if (!checkEsp(rtc_gpio_pulldown_dis(BUTTON_PIN), "RTC pull-down disable")) return;
    if (!checkEsp(esp_sleep_enable_ext0_wakeup(BUTTON_PIN, 0), "EXT0 wakeup")) return;
#endif
    digitalWrite(LED_PIN, LOW);
    Serial.printf("SLEEP: RTC=%lu normal=%lu; timer=10s EXT0=%d\n",
                  static_cast<unsigned long>(rtcWakeCount),
                  static_cast<unsigned long>(normalWakeCount), ENABLE_EXT0);
    Serial.println("Khi thuc: chuong trinh se chay lai tu setup().");
    Serial.flush();
    esp_deep_sleep_start();
    // Ham tren khong return. Sau khi thuc, boot/ setup chay lai.
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
#if ENABLE_EXT0
    // Sau EXT0 wake, tra chan tu RTC GPIO ve digital GPIO truoc khi doc nut.
    if (!checkEsp(rtc_gpio_deinit(BUTTON_PIN), "RTC GPIO deinit")) return;
    pinMode(static_cast<uint8_t>(BUTTON_PIN), INPUT_PULLUP);
#endif
    ++rtcWakeCount;
    ++normalWakeCount;
    const auto cause = esp_sleep_get_wakeup_cause();
    Serial.println("\nSETUP: bat dau mot lan boot/thuc day");
    Serial.printf("cause=%s (%d) RTC=%lu normal=%lu\n", wakeName(cause),
                  static_cast<int>(cause), static_cast<unsigned long>(rtcWakeCount),
                  static_cast<unsigned long>(normalWakeCount));
    Serial.println("Thuc 5s, LED chop; sau do ngu 10s. Khong can Wi-Fi.");
    bootMs = lastLedMs = millis();
}

void loop() {
    if (sleepFailed) return;
    const uint32_t now = millis();
    if (now - lastLedMs >= 500) {
        lastLedMs = now;
        ledOn = !ledOn;
        digitalWrite(LED_PIN, ledOn);
        Serial.printf(">led:%d\n>rtc:%lu\n>normal:%lu\n", ledOn,
                      static_cast<unsigned long>(rtcWakeCount),
                      static_cast<unsigned long>(normalWakeCount));
    }
    if (now - bootMs < AWAKE_MS) return;
#if ENABLE_EXT0
    if (digitalRead(static_cast<uint8_t>(BUTTON_PIN)) == LOW) {
        if (!releaseMessagePrinted) Serial.println("Nha nut GPIO4 de vao deep sleep.");
        releaseMessagePrinted = true;
        return;
    }
    // Doi HIGH on dinh 50ms sau khi nha nut (khong delay loop).
    static uint32_t highSinceMs = 0;
    if (releaseMessagePrinted) {
        highSinceMs = now;
        releaseMessagePrinted = false;
    }
    if (now - highSinceMs < 50) return;
#endif
    enterDeepSleep();
}
