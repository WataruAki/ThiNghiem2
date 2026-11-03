#include <cassert>
#include "timing_stats.h"

int main() {
    TimingStats stats;
    stats.record(100, 500000);
    assert(stats.samples == 0);
    stats.record(500100, 500000);
    assert(stats.intervalUs == 500000 && stats.errorUs == 0);
    stats.record(1000200, 500000);
    assert(stats.errorUs == 100);
    stats.record(1500100, 500000);
    assert(stats.errorUs == -100);
    assert(stats.minimumUs == 499900 && stats.maximumUs == 500100);
    assert(stats.absoluteErrorSum == 200 && stats.samples == 3);
    assert(stats.meanAbsoluteErrorUs() > 66.66 && stats.meanAbsoluteErrorUs() < 66.67);

    TimingStats rollover;
    rollover.record(UINT32_MAX - 199, 500000);
    rollover.record(499800, 500000);
    assert(rollover.intervalUs == 500000 && rollover.errorUs == 0);

    TimingStats blocking;
    blocking.record(0, 500000);
    blocking.record(500000, 500000);
    blocking.record(8000000, 500000);
    assert(blocking.intervalUs == 7500000 && blocking.errorUs == 7000000);
    assert(blocking.meanAbsoluteErrorUs() == 3500000.0);
}
