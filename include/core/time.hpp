#pragma once

#include <string>

#include <stdx/option.hh>
#include <stdx/types.hh>

namespace mbr {

struct local_time {
    explicit local_time();
    explicit local_time(u64 hour, u64 minute, u64 second, u64 millisecond, u64 microsecond);
    explicit local_time(u64 micros);

    [[nodiscard]] static local_time zero() noexcept;

    [[nodiscard]] u64    micros_since_midnight() const;
    [[nodiscard]] double minutes_since_midnight() const;

    // Tries to parse a string formatted as "HH:MM:SS".
    [[nodiscard]] static stdx::option<local_time> from_string(const std::string& input);
    [[nodiscard]] static stdx::option<local_time> from_minutes(double minutes);

    u64 hour;
    u64 minute;
    u64 second;
    u64 millisecond;
    u64 microsecond;

    [[nodiscard]] std::string to_string(bool high_precision = true) const;
};

struct date_time {
    explicit date_time();
    explicit date_time(u64 creation_time_seconds);

    enum class fmt_t : u8 {
        DISPLAY,
        TEXT_FILE,
    };

    u64        year;
    u64        month;
    u64        day;
    local_time local;

    [[nodiscard]] static stdx::option<date_time> from_video_metadata(const std::string& path);
    [[nodiscard]] std::string                    to_string(fmt_t fmt = fmt_t::DISPLAY) const;
};

} // namespace mbr
