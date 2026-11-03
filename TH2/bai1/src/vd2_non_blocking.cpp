#include <Arduino.h>
#include <stdint.h>

// Unsigned subtraction remains valid across micros() rollover, provided
// consecutive samples are less than one full 32-bit counter cycle apart.
struct TimingStats {
    uint32_t previousUs = 0;
    uint32_t samples = 0;
    uint32_t intervalUs = 0;
    int32_t errorUs = 0;
    uint32_t minimumUs = UINT32_MAX;
    uint32_t maximumUs = 0;
    uint64_t absoluteErrorSum = 0;
    bool started = false;

    void record(uint32_t nowUs, uint32_t targetUs) {
        if (!started) {
            previousUs = nowUs;
            started = true;
            return;
        }
        intervalUs = nowUs - previousUs;
        previousUs = nowUs;
        errorUs = static_cast<int32_t>(static_cast<int64_t>(intervalUs) - targetUs);
        minimumUs = intervalUs < minimumUs ? intervalUs : minimumUs;
        maximumUs = intervalUs > maximumUs ? intervalUs : maximumUs;
        absoluteErrorSum += errorUs < 0 ? -static_cast<int64_t>(errorUs) : errorUs;
        ++samples;
    }

    double meanAbsoluteErrorUs() const {
        return samples == 0 ? 0.0 : static_cast<double>(absoluteErrorSum) / samples;
    }
};


#ifndef BUSY_DEMO
#define BUSY_DEMO 0
#endif

constexpr uint8_t kLedPin = 2;
constexpr uint8_t kAdcPin = 1;  // ESP32-S3 ADC1_CH0; NOT GPIO34 from classic ESP32.
constexpr uint32_t kLedMs = 500;
constexpr uint32_t kAdcMs = 2000;
constexpr uint32_t kHeartMs = 5000;

inline void beginLab(const char* mode) {
    Serial.begin(115200);
    pinMode(kLedPin, OUTPUT);
    digitalWrite(kLedPin, LOW);
    analogReadResolution(12);
    Serial.printf("# BAI1 mode=%s busy=%d LED=GPIO2 ADC=GPIO1\n", mode, BUSY_DEMO);
    Serial.println("# dt_us=actual interval; error_us=dt_us-target_us; mae_us=mean(abs(error_us))");
}

inline void printStats(const char* name, const TimingStats& stats) {
    if (stats.samples == 0) return;  // First event establishes the timestamp.
    Serial.printf(">%s_dt_us:%lu\n>%s_error_us:%ld\n>%s_mae_us:%.2f\n",
                  name, static_cast<unsigned long>(stats.intervalUs),
                  name, static_cast<long>(stats.errorUs),
                  name, stats.meanAbsoluteErrorUs());
    Serial.printf("# %s n=%lu min_us=%lu max_us=%lu\n", name,
                  static_cast<unsigned long>(stats.samples),
                  static_cast<unsigned long>(stats.minimumUs),
                  static_cast<unsigned long>(stats.maximumUs));
}

inline void toggleLed() {
    static bool on = false;
    static TimingStats stats;
    on = !on;
    digitalWrite(kLedPin, on);
    stats.record(micros(), kLedMs * 1000);
    Serial.printf(">led:%d\n", on);
    printStats("led", stats);
}

inline void readAdc() {
    static TimingStats stats;
    stats.record(micros(), kAdcMs * 1000);
    Serial.printf(">bienTro:%d\n", analogRead(kAdcPin));
    printStats("adc", stats);
}

inline void heartbeat() {
    static TimingStats stats;
    static uint32_t count = 0;
    stats.record(micros(), kHeartMs * 1000);
    Serial.printf(">heartbeat:%lu\n", static_cast<unsigned long>(++count));
    printStats("heart", stats);
}

// Occupy loop() for 1200 ms every ~3 s. Interrupts remain ENABLED:
// this compares loop scheduling with hardware interrupts, not interrupt masking.
inline void simulateBusyLoop() {
    if (!BUSY_DEMO) return;
    static uint32_t lastBusyMs = millis();
    const uint32_t nowMs = millis();
    if (nowMs - lastBusyMs < 3000) return;
    lastBusyMs = nowMs;
    const uint32_t startUs = micros();
    while (static_cast<uint32_t>(micros() - startUs) < 1200000UL) {
        asm volatile("nop");
    }
}


uint32_t lastLedMs = 0;
uint32_t lastAdcMs = 0;
uint32_t lastHeartMs = 0;

// Keep the nominal phase. When late, execute once, skip missed periods,
// and expose the delay in measured intervals instead of fabricating catch-up events.
bool isDue(uint32_t nowMs, uint32_t& lastMs, uint32_t periodMs) {
    const uint32_t elapsedMs = nowMs - lastMs;
    if (elapsedMs < periodMs) return false;
    lastMs += (elapsedMs / periodMs) * periodMs;
    return true;
}

void setup() {
    beginLab("millis");
    lastLedMs = lastAdcMs = lastHeartMs = millis();
}

void loop() {
    simulateBusyLoop();
    if (isDue(millis(), lastLedMs, kLedMs)) toggleLed();
    if (isDue(millis(), lastAdcMs, kAdcMs)) readAdc();
    if (isDue(millis(), lastHeartMs, kHeartMs)) heartbeat();
}
