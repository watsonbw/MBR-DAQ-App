#include <cassert>
#include <chrono>
#include <format>
#include <sstream>

#include <taglib/mp4/mp4file.h>

#include "core/time.hpp"

using namespace std::chrono;

const uint64_t UNIX_1904_DIFF = 2082844800ULL;

LocalTime::LocalTime() {
    const auto now = system_clock::now();

    const auto time_now = system_clock::to_time_t(now);
    std::tm    lt{};
#ifdef _WIN32
    localtime_s(&lt, &time_now);
#else
    localtime_r(&time_now, &lt);
#endif

    Hour   = static_cast<uint64_t>(lt.tm_hour);
    Minute = static_cast<uint64_t>(lt.tm_min);
    Second = static_cast<uint64_t>(lt.tm_sec);

    const auto duration = now.time_since_epoch();
    auto       ms       = duration_cast<milliseconds>(duration) % 1000;
    Millisecond         = static_cast<uint64_t>(ms.count());

    auto us     = duration_cast<microseconds>(duration) % 1000;
    Microsecond = static_cast<uint64_t>(us.count());
}

LocalTime::LocalTime(
    uint64_t hour, uint64_t minute, uint64_t second, uint64_t millisecond, uint64_t microsecond)
    : Hour{hour}, Minute{minute}, Second{second}, Millisecond{millisecond},
      Microsecond{microsecond} {}

LocalTime::LocalTime(uint64_t micros) {
    Microsecond                  = micros % 1000;
    const uint64_t total_ms      = micros / 1000;
    Millisecond                  = total_ms % 1000;
    const uint64_t total_seconds = total_ms / 1000;
    Second                       = total_seconds % 60;
    const uint64_t total_minutes = total_seconds / 60;
    Minute                       = total_minutes % 60;
    const uint64_t total_hours   = total_minutes / 60;
    Hour                         = total_hours % 24;
}

LocalTime LocalTime::Zero() noexcept { return LocalTime{0, 0, 0, 0, 0}; }

uint64_t LocalTime::MicrosSinceMidnight() const {
    uint64_t acc = 0;
    acc += Hour * 3600000000;
    acc += Minute * 60000000;
    acc += Second * 1000000;
    acc += Millisecond * 1000;
    acc += Microsecond;
    return acc;
}

double LocalTime::MinutesSinceMidnight() const {
    double acc = 0;
    acc += static_cast<double>(Hour) * 60.0;
    acc += static_cast<double>(Minute);
    acc += static_cast<double>(Second) / 60.0;
    acc += static_cast<double>(Millisecond) / 60'000.0;
    acc += static_cast<double>(Microsecond) / 60'000'000.0;
    return acc;
}

std::string LocalTime::String(bool high_precision) const {
    if (high_precision) {
        return std::format(
            "{:02}:{:02}:{:02}.{:03}{:03}", Hour, Minute, Second, Millisecond, Microsecond);
    }
    return std::format("{:02}:{:02}:{:02}", Hour, Minute, Second);
}

std::optional<LocalTime> LocalTime::FromString(const std::string& input) {
    std::istringstream ss{input};
    int                h, m, s;
    char               c1, c2;

    if (ss >> h >> c1 >> m >> c2 >> s) {
        if (c1 == ':' && c2 == ':' && ss.eof()) {
            if (h >= 0 && h < 24 && m >= 0 && m < 60 && s >= 0 && s < 60) {
                return LocalTime{static_cast<uint64_t>(h),
                                 static_cast<uint64_t>(m),
                                 static_cast<uint64_t>(s),
                                 0,
                                 0};
            }
        }
    }
    return std::nullopt;
}

std::optional<LocalTime> LocalTime::FromMinutes(double minutes) {
    if (minutes < 0) { return std::nullopt; }
    const auto micros = static_cast<uint64_t>(minutes * 60'000'000.0);
    return LocalTime{micros};
}

DateTime::DateTime() {
    auto now      = system_clock::now();
    auto duration = now.time_since_epoch();

    const auto time_now = system_clock::to_time_t(now);
    std::tm    lt{};
#ifdef _WIN32
    localtime_s(&lt, &time_now);
#else
    localtime_r(&time_now, &lt);
#endif

    Year  = lt.tm_year + 1900;
    Month = lt.tm_mon + 1;
    Day   = lt.tm_mday;

    const auto total_us = duration_cast<microseconds>(duration).count();
    Local               = LocalTime{static_cast<uint64_t>(lt.tm_hour),
                      static_cast<uint64_t>(lt.tm_min),
                      static_cast<uint64_t>(lt.tm_sec),
                      static_cast<uint64_t>((total_us / 1000) % 1000),
                      static_cast<uint64_t>(total_us % 1000)};
}

DateTime::DateTime(uint64_t creation_time_seconds) {
    const uint64_t unix_seconds = creation_time_seconds - UNIX_1904_DIFF;

    const auto time_now = static_cast<time_t>(unix_seconds);
    std::tm    lt{};
#ifdef _WIN32
    localtime_s(&lt, &time_now);
#else
    localtime_r(&time_now, &lt);
#endif

    Year  = lt.tm_year + 1900;
    Month = lt.tm_mon + 1;
    Day   = lt.tm_mday;

    // Accuracy is restricted to seconds
    Local = LocalTime{static_cast<uint64_t>(lt.tm_hour),
                      static_cast<uint64_t>(lt.tm_min),
                      static_cast<uint64_t>(lt.tm_sec),
                      0,
                      0};
}

std::optional<DateTime> DateTime::FromVideoMetadata(const std::string& path) {
    TagLib::MP4::File f(path.c_str());
    if (!f.isValid()) { return std::nullopt; }

    // Search the first 100KB for the desired metadata block
    f.seek(0);
    TagLib::ByteVector data = f.readBlock(static_cast<size_t>(100 * 1024));

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

    if (creation_time_seconds > UNIX_1904_DIFF) { return DateTime{creation_time_seconds}; }
    return std::nullopt;
}

std::string DateTime::String(StringFormat fmt) const {
    switch (fmt) {
    case StringFormat::DISPLAY:
        return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}{:03}",
                           Year,
                           Month,
                           Day,
                           Local.Hour,
                           Local.Minute,
                           Local.Second,
                           Local.Millisecond,
                           Local.Microsecond);
    case StringFormat::TEXT_FILE:
        return std::format("{:04}-{:02}-{:02}_{:02}-{:02}-{:02}",
                           Year,
                           Month,
                           Day,
                           Local.Hour,
                           Local.Minute,
                           Local.Second);
    default:
        return std::string{};
    }
}
