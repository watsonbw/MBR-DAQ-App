#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct LocalTime {
    explicit LocalTime();
    explicit LocalTime(uint64_t micros);

    uint64_t MicrosSinceMidnight() const;

    uint64_t Hour;
    uint64_t Minute;
    uint64_t Second;
    uint64_t Millisecond;
    uint64_t Microsecond;
};

struct DateTime {
    explicit DateTime();
    explicit DateTime(uint64_t creation_time_seconds);

    uint64_t  Year;
    uint64_t  Month;
    uint64_t  Day;
    LocalTime Local;

    static std::optional<DateTime> FromVideoMetadata(const std::string& path);
    std::string                    String() const;
    std::string                    txtString() const;
};
