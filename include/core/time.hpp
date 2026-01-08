#pragma once

#include <cstdint>
#include <optional>
#include <string>

struct LocalTime {
    explicit LocalTime();
    explicit LocalTime(uint64_t hour,
                       uint64_t minute,
                       uint64_t second,
                       uint64_t millisecond,
                       uint64_t microsecond);
    explicit LocalTime(uint64_t micros);

    [[nodiscard]] static LocalTime Zero() noexcept;

    [[nodiscard]] uint64_t MicrosSinceMidnight() const;
    [[nodiscard]] double   MinutesSinceMidnight() const;

    // Tries to parse a string formatted as "HH:MM:SS".
    [[nodiscard]] static std::optional<LocalTime> FromString(const std::string& input);
    [[nodiscard]] static std::optional<LocalTime> FromMinutes(double minutes);

    uint64_t Hour;
    uint64_t Minute;
    uint64_t Second;
    uint64_t Millisecond;
    uint64_t Microsecond;

    [[nodiscard]] std::string String(bool high_precision = true) const;
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
