#pragma once

#include <cstdint>

struct LocalTime {
    explicit LocalTime();

    int     Hour;
    int     Minute;
    int     Second;
    int64_t Millisecond;
};
