#include <chrono>
#include <format>

#include <taglib/mp4/mp4file.h>

#include "core/time.hpp"

using namespace std::chrono;

const uint64_t UNIX_1904_DIFF = 2082844800ULL;

LocalTime::LocalTime() {
#ifdef __APPLE__
    const auto now = system_clock::now();

    auto    time_now = system_clock::to_time_t(now);
    std::tm lt{};
    localtime_r(&time_now, &lt);

    Hour   = static_cast<uint64_t>(lt.tm_hour);
    Minute = static_cast<uint64_t>(lt.tm_min);
    Second = static_cast<uint64_t>(lt.tm_sec);

    const auto duration = now.time_since_epoch();
    auto       ms       = duration_cast<milliseconds>(duration) % 1000;
    Millisecond         = static_cast<uint64_t>(ms.count());

    auto us     = duration_cast<microseconds>(duration) % 1000;
    Microsecond = static_cast<uint64_t>(us.count());
#else
    // https://stackoverflow.com/questions/61273498/number-of-seconds-since-midnight
    auto now            = current_zone()->to_local(system_clock::now());
    auto today          = floor<days>(now);
    auto since_midnight = duration_cast<milliseconds>(now - today);

    Hour = static_cast<uint64_t>(duration_cast<hours>(since_midnight).count());
    since_midnight -= hours{Hour};

    Minute = static_cast<uint64_t>(duration_cast<minutes>(since_midnight).count());
    since_midnight -= minutes{Minute};

    Second = static_cast<uint64_t>(duration_cast<seconds>(since_midnight).count());
    since_midnight -= seconds{Second};

    const auto total_us = duration_cast<microseconds>(now.time_since_epoch());
    Millisecond         = static_cast<uint64_t>((total_us.count() / 1000) % 1000);
    Microsecond         = static_cast<uint64_t>(total_us.count() % 1000);
#endif
}

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

uint64_t LocalTime::MicrosSinceMidnight() const {
    uint64_t acc = 0;
    acc += Hour * 3600000000;
    acc += Minute * 60000000;
    acc += Second * 1000000;
    acc += Millisecond * 1000;
    acc += Microsecond;
    return acc;
}

DateTime::DateTime() {
    auto now      = system_clock::now();
    auto duration = now.time_since_epoch();

#ifdef __APPLE__
    time_t  time_now = system_clock::to_time_t(now);
    std::tm lt_struct{};
    localtime_r(&time_now, &lt_struct);

    Year  = lt_struct.tm_year + 1900;
    Month = lt_struct.tm_mon + 1;
    Day   = lt_struct.tm_mday;

    Local.Hour   = lt_struct.tm_hour;
    Local.Minute = lt_struct.tm_min;
    Local.Second = lt_struct.tm_sec;
#else
    auto           local_now = current_zone()->to_local(now);
    auto           day_point = floor<days>(local_now);
    year_month_day ymd{day_point};

    Year  = static_cast<int>(ymd.year());
    Month = static_cast<unsigned>(ymd.month());
    Day   = static_cast<unsigned>(ymd.day());

    auto tod     = local_now - day_point;
    auto hms     = hh_mm_ss(tod);
    Local.Hour   = hms.hours().count();
    Local.Minute = hms.minutes().count();
    Local.Second = hms.seconds().count();
#endif

    // Platform independent time points
    auto total_us     = duration_cast<microseconds>(duration).count();
    Local.Millisecond = (total_us / 1000) % 1000;
    Local.Microsecond = total_us % 1000;
}

DateTime::DateTime(uint64_t creation_time_seconds) {
    uint64_t unix_seconds = creation_time_seconds - UNIX_1904_DIFF;

#ifdef __APPLE__
    time_t  t = static_cast<time_t>(unix_seconds);
    std::tm lt_struct{};
    localtime_r(&t, &lt_struct);

    Year  = lt_struct.tm_year + 1900;
    Month = lt_struct.tm_mon + 1;
    Day   = lt_struct.tm_mday;

    Local.Hour   = lt_struct.tm_hour;
    Local.Minute = lt_struct.tm_min;
    Local.Second = lt_struct.tm_sec;
#else
    auto           tp_utc    = sys_seconds{seconds(unix_seconds)};
    auto           local_tp  = current_zone()->to_local(tp_utc);
    auto           day_point = floor<days>(local_tp);
    year_month_day ymd{day_point};

    Year  = static_cast<int>(ymd.year());
    Month = static_cast<unsigned>(ymd.month());
    Day   = static_cast<unsigned>(ymd.day());

    auto tod = duration_cast<microseconds>(local_tp - day_point);
    Local    = LocalTime{static_cast<uint64_t>(tod.count())};
#endif

    // Accuracy is restricted to seconds
    Local.Millisecond = 0;
    Local.Microsecond = 0;
}

std::optional<DateTime> DateTime::FromVideoMetadata(const std::string& path) {
    TagLib::MP4::File f(path.c_str());
    if (!f.isValid()) { return std::nullopt; }

    // Search the first 100KB for the desired metadata block
    f.seek(0);
    TagLib::ByteVector data = f.readBlock(100 * 1024);

    int pos = data.find("mvhd");
    if (pos == -1) { return std::nullopt; }

    // Decode the raw metadata based on header version
    unsigned char version               = data[pos + 4];
    uint64_t      creation_time_seconds = 0;

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

std::string DateTime::String() const {
    return std::format("{:04}-{:02}-{:02} {:02}:{:02}:{:02}.{:03}{:03}",
                       Year,
                       Month,
                       Day,
                       Local.Hour,
                       Local.Minute,
                       Local.Second,
                       Local.Millisecond,
                       Local.Microsecond);
}
