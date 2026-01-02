#include <chrono>

#include "core/time.hpp"

using namespace std::chrono;

LocalTime::LocalTime() {
#ifdef __APPLE__
    auto now = system_clock::now();

    auto    time_now = system_clock::to_time_t(now);
    std::tm lt{};
    localtime_r(&time_now, &lt);

    Hour   = static_cast<uint64_t>(lt.tm_hour);
    Minute = static_cast<uint64_t>(lt.tm_min);
    Second = static_cast<uint64_t>(lt.tm_sec);

    auto ms     = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    Millisecond = static_cast<uint64_t>(ms.count());

    auto us     = duration_cast<microseconds>(now.time_since_epoch()) % 1000000;
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
