#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct LocalTime {
    explicit LocalTime();
    explicit LocalTime(uint64_t micros);

    [[nodiscard]] uint64_t MicrosSinceMidnight() const;
    [[nodiscard]] double   MinutesSinceMidnight() const;

    // Tries to parse a string formatted as "HH:MM:SS".
    [[nodiscard]] static std::optional<LocalTime> FromString(const std::string& input);

    uint64_t Hour;
    uint64_t Minute;
    uint64_t Second;
    uint64_t Millisecond;
    uint64_t Microsecond;

    [[nodiscard]] std::string String() const;
};

struct DateTime {
    explicit DateTime();
    explicit DateTime(uint64_t creation_time_seconds);

    enum class StringFormat : uint8_t {
        DISPLAY,
        TEXT_FILE,
    };

    uint64_t  Year;
    uint64_t  Month;
    uint64_t  Day;
    LocalTime Local;

    [[nodiscard]] static std::optional<DateTime> FromVideoMetadata(const std::string& path);
    [[nodiscard]] std::string String(StringFormat fmt = StringFormat::DISPLAY) const;
};
