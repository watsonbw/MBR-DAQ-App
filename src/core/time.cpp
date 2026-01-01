#include <chrono>

#include "core/time.hpp"

using namespace std::chrono;

LocalTime::LocalTime() {
#ifdef __APPLE__
    auto now = system_clock::now();

    auto    time_now = system_clock::to_time_t(now);
    std::tm lt{};
    localtime_r(&time_now, &lt);

    Hour        = lt.tm_hour;
    Minute      = lt.tm_min;
    Second      = lt.tm_sec;
    auto ms     = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;
    Millisecond = ms.count();
#else
    // https://stackoverflow.com/questions/61273498/number-of-seconds-since-midnight
    auto now            = current_zone()->to_local(system_clock::now());
    auto today          = floor<days>(now);
    auto since_midnight = duration_cast<milliseconds>(now - today);

    Hour = duration_cast<hours>(since_midnight).count();
    since_midnight -= hours{Hour};

    Minute = duration_cast<minutes>(since_midnight).count();
    since_midnight -= minutes{Minute};

    Second = duration_cast<seconds>(since_midnight).count();
    since_midnight -= seconds{Second};

    Millisecond = since_midnight.count();
#endif
}
