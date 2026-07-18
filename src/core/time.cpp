#include <cassert>
#include <chrono>
#include <sstream>

#include <fmt/format.h>
#include <taglib/mp4/mp4file.h>

#include "core/time.hpp"

using namespace std::chrono;

namespace mbr {

const uint64_t UNIX_1904_DIFF = 2'082'844'800ULL;

local_time::local_time() {
    const auto now = system_clock::now();

    const auto time_now = system_clock::to_time_t(now);
    std::tm    lt{};
#ifdef _WIN32
    localtime_s(&lt, &time_now);
#else
    localtime_r(&time_now, &lt);
#endif

    hour   = static_cast<uint64_t>(lt.tm_hour);
    minute = static_cast<uint64_t>(lt.tm_min);
    second = static_cast<uint64_t>(lt.tm_sec);

    const auto duration = now.time_since_epoch();
    auto       ms       = duration_cast<milliseconds>(duration) % 1'000;
    millisecond         = static_cast<uint64_t>(ms.count());

    auto us     = duration_cast<microseconds>(duration) % 1'000;
    microsecond = static_cast<uint64_t>(us.count());
}

local_time::local_time(
    uint64_t hour, uint64_t minute, uint64_t second, uint64_t millisecond, uint64_t microsecond)
    : hour{hour}, minute{minute}, second{second}, millisecond{millisecond},
      microsecond{microsecond} {}

local_time::local_time(uint64_t micros) {
    microsecond                  = micros % 1'000;
    const uint64_t total_ms      = micros / 1'000;
    millisecond                  = total_ms % 1'000;
    const uint64_t total_seconds = total_ms / 1'000;
    second                       = total_seconds % 60;
    const uint64_t total_minutes = total_seconds / 60;
    minute                       = total_minutes % 60;
    const uint64_t total_hours   = total_minutes / 60;
    hour                         = total_hours % 24;
}

local_time local_time::zero() noexcept { return local_time{0, 0, 0, 0, 0}; }

uint64_t local_time::micros_since_midnight() const {
    uint64_t acc = 0;
    acc += hour * 3'600'000'000;
    acc += minute * 60'000'000;
    acc += second * 1'000'000;
    acc += millisecond * 1'000;
    acc += microsecond;
    return acc;
}

double local_time::minutes_since_midnight() const {
    double acc = 0;
    acc += static_cast<double>(hour) * 60.0;
    acc += static_cast<double>(minute);
    acc += static_cast<double>(second) / 60.0;
    acc += static_cast<double>(millisecond) / 60'000.0;
    acc += static_cast<double>(microsecond) / 60'000'000.0;
    return acc;
}

std::string local_time::to_string(bool high_precision) const {
    if (high_precision) {
        return fmt::format(
            "{:02}:{:02}:{:02}.{:03}{:03}", hour, minute, second, millisecond, microsecond);
    }
    return fmt::format("{:02}:{:02}:{:02}", hour, minute, second);
}

std::optional<local_time> local_time::from_string(const std::string& input) {
    std::istringstream ss{input};
    int                h, m, s;
    char               c1, c2;

    if (ss >> h >> c1 >> m >> c2 >> s) {
        if (c1 == ':' && c2 == ':' && ss.eof()) {
            if (h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60) {
                return local_time{static_cast<uint64_t>(h),
                                  static_cast<uint64_t>(m),
                                  static_cast<uint64_t>(s),
                                  0,
                                  0};
            }
        }
    }
    return std::nullopt;
}

std::optional<local_time> local_time::from_minutes(double minutes) {
    if (minutes < 0) { return std::nullopt; }
    const auto micros = static_cast<uint64_t>(minutes * 60'000'000.0);
    return local_time{micros};
}

date_time::date_time() {
    auto now      = system_clock::now();
    auto duration = now.time_since_epoch();

    const auto time_now = system_clock::to_time_t(now);
    std::tm    lt{};
#ifdef _WIN32
    localtime_s(&lt, &time_now);
#else
    localtime_r(&time_now, &lt);
#endif

    year  = lt.tm_year + 1'900;
    month = lt.tm_mon + 1;
    day   = lt.tm_mday;

    const auto total_us = duration_cast<microseconds>(duration).count();
    local               = local_time{static_cast<uint64_t>(lt.tm_hour),
                       static_cast<uint64_t>(lt.tm_min),
                       static_cast<uint64_t>(lt.tm_sec),
                       static_cast<uint64_t>((total_us / 1'000) % 1'000),
                       static_cast<uint64_t>(total_us % 1'000)};
}

date_time::date_time(uint64_t creation_time_seconds) {
    const uint64_t unix_seconds = creation_time_seconds - UNIX_1904_DIFF;

    const auto time_now = static_cast<time_t>(unix_seconds);
    std::tm    lt{};
#ifdef _WIN32
    localtime_s(&lt, &time_now);
#else
    localtime_r(&time_now, &lt);
#endif

    year  = lt.tm_year + 1'900;
    month = lt.tm_mon + 1;
    day   = lt.tm_mday;

    // Accuracy is restricted to seconds
    local = local_time{static_cast<uint64_t>(lt.tm_hour),
                       static_cast<uint64_t>(lt.tm_min),
                       static_cast<uint64_t>(lt.tm_sec),
                       0,
                       0};
}

std::optional<date_time> date_time::from_video_metadata(const std::string& path) {
    TagLib::MP4::File f(path.c_str());
    if (!f.isValid()) { return std::nullopt; }

    // Search the first 100KB for the desired metadata block
    f.seek(0);
    TagLib::ByteVector data = f.readBlock(static_cast<size_t>(100 * 1'024));

    const int pos = data.find("mvhd");
    if (pos == -1) { return std::nullopt; }

    // Decode the raw metadata based on header version
    const unsigned char version               = data[pos + 4];
    uint64_t            creation_time_seconds = 0;

    // v0 == 32 bit, v1 == 64 bit (different shift values too)
    if (version == 0) {
        uint32_t t = 0;
        t |= static_cast<unsigned char>(data[pos + 8]) << 24;
        t |= static_cast<unsigned char>(data[pos + 9]) << 16;
        t |= static_cast<unsigned char>(data[pos + 10]) << 8;
        t |= static_cast<unsigned char>(data[pos + 11]);
        creation_time_seconds = t;
    } else if (version == 1) {
        for (int i = 0; i < 8; ++i) {
            creation_time_seconds |=
                static_cast<uint64_t>(static_cast<unsigned char>(data[pos + 8 + i]))
                << (56 - (i * 8));
        }
    }

    if (creation_time_seconds > UNIX_1904_DIFF) { return date_time{creation_time_seconds}; }
    return std::nullopt;
}

std::string date_time::to_string(fmt_t fmt) const {
    switch (fmt) {
    case fmt_t::DISPLAY:
        return fmt::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}{:03}",
                           year,
                           month,
                           day,
                           local.hour,
                           local.minute,
                           local.second,
                           local.millisecond,
                           local.microsecond);
    case fmt_t::TEXT_FILE:
        return fmt::format("{:04}-{:02}-{:02}_{:02}-{:02}-{:02}",
                           year,
                           month,
                           day,
                           local.hour,
                           local.minute,
                           local.second);
    default: return std::string{};
    }
}

} // namespace mbr
