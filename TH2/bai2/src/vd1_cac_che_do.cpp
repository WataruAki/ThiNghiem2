#include <Arduino.h>
#include <WiFi.h>
#include <esp_sleep.h>
#include <esp_timer.h>
#include <esp_wifi.h>

constexpr uint8_t LED_PIN = 2;  // LED roi, khong phai RGB tren board.
// Dien Wi-Fi 2.4 GHz cua ban de thu modem-sleep. Khong commit mat khau that.
constexpr char WIFI_SSID[] = "";
constexpr char WIFI_PASSWORD[] = "";
constexpr uint64_t LIGHT_SLEEP_US = 5ULL * 1000000ULL;

uint32_t heartbeatCount = 0;
uint32_t lastBeatMs = 0;
bool ledOn = false;
bool modemRequested = false;
bool connectionReported = false;

bool checkEsp(esp_err_t error, const char* operation) {
    if (error == ESP_OK) return true;
    Serial.printf("ERROR %s: %s\n", operation, esp_err_to_name(error));
    return false;
}

void printHelp() {
    Serial.println("\nBAI 2 VD1: a=active, m=modem-sleep, l=light-sleep 5s, h=help");
    Serial.println("LED/heartbeat cho biet loop con chay; KHONG phai so do dong dien.");
    Serial.println("Light sleep se ngat Wi-Fi; reset de ket noi lai.");
}

void selectWifiPowerMode(bool modemSleep) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi chua ket noi: chua the xac nhan thu modem-sleep.");
        Serial.println("De active/modem co cung dieu kien, dien SSID/password roi reset.");
        return;
    }
    const wifi_ps_type_t requested = modemSleep ? WIFI_PS_MIN_MODEM : WIFI_PS_NONE;
    if (!checkEsp(esp_wifi_set_ps(requested), "set Wi-Fi PS")) return;
    wifi_ps_type_t actual;
    if (!checkEsp(esp_wifi_get_ps(&actual), "get Wi-Fi PS")) return;
    if (actual != requested) {
        Serial.println("ERROR: driver khong giu che do vua yeu cau");
        return;
    }
    modemRequested = modemSleep;
    Serial.printf("CONFIGURED: %s; CPU van chay. Chua do dong dien.\n",
                  modemSleep ? "MODEM (MIN_MODEM)" : "ACTIVE (PS_NONE)");
}

void tryLightSleep() {
    // Wi-Fi/BT phai dung truoc manual light sleep. Vi du nay khong khoi tao BT.
    if (!WiFi.mode(WIFI_OFF)) {
        Serial.println("ERROR: khong tat duoc Wi-Fi, huy thu light sleep");
        return;
    }
    connectionReported = false;
    modemRequested = false;
    if (!checkEsp(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL), "clear wakeup")) return;
    if (!checkEsp(esp_sleep_enable_timer_wakeup(LIGHT_SLEEP_US), "timer wakeup")) return;

    int localCounter = 41;  // RAM thuong, khong co RTC_DATA_ATTR.
    const uint32_t beforeHeartbeat = heartbeatCount;
    digitalWrite(LED_PIN, LOW);
    ledOn = false;
    Serial.printf("BEFORE light: local=%d heartbeat=%lu; sleep=5s\n",
                  localCounter, static_cast<unsigned long>(beforeHeartbeat));
    Serial.flush();
    const int64_t startUs = esp_timer_get_time();
    const esp_err_t result = esp_light_sleep_start();
    const int64_t elapsedUs = esp_timer_get_time() - startUs;
    if (!checkEsp(result, "light sleep")) {
        esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);
        return;
    }
    ++localCounter;
    Serial.printf("AFTER light: local=%d heartbeat=%lu elapsed_us=%lld cause=%d\n",
                  localCounter, static_cast<unsigned long>(heartbeatCount),
                  static_cast<long long>(elapsedUs),
                  static_cast<int>(esp_sleep_get_wakeup_cause()));
    Serial.println("Da tiep tuc NGAY SAU ham sleep; setup khong chay lai; RAM van giu.");
    checkEsp(esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL), "clear wakeup after light");
    lastBeatMs = millis();
}

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);
    Serial.println("SETUP: khoi dong VD1 (chi in lai khi reset)");
    printHelp();
    if (WIFI_SSID[0] == '\0') {
        WiFi.mode(WIFI_OFF);
        Serial.println("Chua dien Wi-Fi: chay active radio-off va light sleep duoc.");
        Serial.println("Modem-sleep: NOT TESTED cho toi khi co ket noi AP.");
        return;
    }
    if (!WiFi.mode(WIFI_STA)) {
        Serial.println("ERROR: khong khoi tao duoc Wi-Fi STA");
        return;
    }
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.println("Dang ket noi Wi-Fi (heartbeat van chay)...");
}

void loop() {
    if (WiFi.status() == WL_CONNECTED && !connectionReported) {
        connectionReported = true;
        Serial.print("Wi-Fi IP: ");
        Serial.println(WiFi.localIP());
        selectWifiPowerMode(false);
    }
    if (WiFi.status() != WL_CONNECTED && connectionReported) {
        connectionReported = false;
        modemRequested = false;
        Serial.println("Wi-Fi mat ket noi; chua con du dieu kien so sanh modem-sleep.");
    }
    if (Serial.available()) {
        const char command = Serial.read();
        switch (command) {
            case 'a': selectWifiPowerMode(false); break;
            case 'm': selectWifiPowerMode(true); break;
            case 'l': tryLightSleep(); break;
            case 'h': printHelp(); break;
            default: break;  // Bo qua CR/LF.
        }
    }
    const uint32_t now = millis();
    if (now - lastBeatMs < 500) return;
    lastBeatMs = now;
    ledOn = !ledOn;
    digitalWrite(LED_PIN, ledOn);
    Serial.printf(">heartbeat:%lu\n>led:%d\n", static_cast<unsigned long>(++heartbeatCount), ledOn);
    Serial.printf("# mode=%s wifi=%s\n", modemRequested ? "MODEM_CONFIGURED" : "ACTIVE",
                  WiFi.status() == WL_CONNECTED ? "connected" : "disconnected");
}
