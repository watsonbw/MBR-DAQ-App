#include <catch_amalgamated.hpp>

#include "core/time.hpp"

namespace mbr::tests {

TEST_CASE("Roundtrip time") {
    const local_time actual;
    const auto       micros = actual.micros_since_midnight();
    const local_time regenerated{micros};

    REQUIRE(actual.hour == regenerated.hour);
    REQUIRE(actual.minute == regenerated.minute);
    REQUIRE(actual.second == regenerated.second);
    REQUIRE(actual.millisecond == regenerated.millisecond);
    REQUIRE(actual.microsecond == regenerated.microsecond);
}

TEST_CASE("local_time parsing and utility functions") {
    SECTION("local_time from valid string") {
        auto parsed = local_time::from_string("12:34:56");
        REQUIRE(parsed.has_value());
        CHECK(parsed->hour == 12);
        CHECK(parsed->minute == 34);
        CHECK(parsed->second == 56);
        CHECK(parsed->millisecond == 0);
        CHECK(parsed->microsecond == 0);
    }

    SECTION("local_time from invalid string formats") {
        CHECK_FALSE(local_time::from_string("12:34").has_value());
        CHECK_FALSE(local_time::from_string("12:34:").has_value());
        CHECK_FALSE(local_time::from_string("12:34:56:78").has_value());
        CHECK_FALSE(local_time::from_string("12:34:5a").has_value());
        CHECK_FALSE(local_time::from_string("24:00:00").has_value());
        CHECK_FALSE(local_time::from_string("12:60:00").has_value());
        CHECK_FALSE(local_time::from_string("12:34:60").has_value());
        CHECK_FALSE(local_time::from_string("-12:34:56").has_value());
    }

    SECTION("local_time from minutes") {
        auto parsed = local_time::from_minutes(125.5);
        REQUIRE(parsed.has_value());
        CHECK(parsed->hour == 2);
        CHECK(parsed->minute == 5);
        CHECK(parsed->second == 30);
        CHECK(parsed->millisecond == 0);
        CHECK(parsed->microsecond == 0);

        CHECK_FALSE(local_time::from_minutes(-10.0).has_value());
    }

    SECTION("local_time to string formatting") {
        local_time t{12, 34, 56, 789, 123};
        CHECK(t.to_string(true) == "12:34:56.789123");
        CHECK(t.to_string(false) == "12:34:56");
    }
}

TEST_CASE("date_time structure and functionality") {
    SECTION("date_time constructor from UNIX 1904 creation seconds") {
        date_time dt{3'818'534'400ULL};
        CHECK(dt.year >= 2'024);
        CHECK(dt.month >= 1);
        CHECK(dt.month <= 12);
        CHECK(dt.day >= 1);
        CHECK(dt.day <= 31);
    }

    SECTION("date_time default constructor") {
        date_time dt;
        CHECK(dt.year >= 2'026);
    }

    SECTION("date_time to_string formatting") {
        date_time dt;
        dt.year  = 2'026;
        dt.month = 7;
        dt.day   = 19;
        dt.local = local_time{15, 40, 18, 789, 123};

        CHECK(dt.to_string(date_time::fmt_t::DISPLAY) == "2026-07-19 15:40:18.789123");
        CHECK(dt.to_string(date_time::fmt_t::TEXT_FILE) == "2026-07-19_15-40-18");
    }
}

} // namespace mbr::tests
