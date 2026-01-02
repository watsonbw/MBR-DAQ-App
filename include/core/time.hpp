#pragma once

#include <cstdint>

struct LocalTime {
    explicit LocalTime();
    explicit LocalTime(uint64_t micros);

    uint64_t MicrosSinceMidnight() const;

    uint64_t     Hour;
    uint64_t     Minute;
    uint64_t     Second;
    uint64_t Millisecond;
    uint64_t Microsecond;
};
