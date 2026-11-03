#pragma once
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
