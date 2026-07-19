#pragma once

#include <cstdint>
#include <string>

#include <stdx/option.hh>

namespace mbr {

struct local_time {
    explicit local_time();
    explicit local_time(uint64_t hour,
                        uint64_t minute,
                        uint64_t second,
                        uint64_t millisecond,
                        uint64_t microsecond);
    explicit local_time(uint64_t micros);

    [[nodiscard]] static local_time zero() noexcept;

    [[nodiscard]] uint64_t micros_since_midnight() const;
    [[nodiscard]] double   minutes_since_midnight() const;

    // Tries to parse a string formatted as "HH:MM:SS".
    [[nodiscard]] static std::optional<local_time> from_string(const std::string& input);
    [[nodiscard]] static std::optional<local_time> from_minutes(double minutes);

    uint64_t hour;
    uint64_t minute;
    uint64_t second;
    uint64_t millisecond;
    uint64_t microsecond;

    [[nodiscard]] std::string to_string(bool high_precision = true) const;
};

struct date_time {
    explicit date_time();
    explicit date_time(uint64_t creation_time_seconds);

    enum class fmt_t : uint8_t {
        DISPLAY,
        TEXT_FILE,
    };

    uint64_t   year;
    uint64_t   month;
    uint64_t   day;
    local_time local;

    [[nodiscard]] static std::optional<date_time> from_video_metadata(const std::string& path);
    [[nodiscard]] std::string                     to_string(fmt_t fmt = fmt_t::DISPLAY) const;
};

} // namespace mbr
