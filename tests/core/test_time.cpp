#include "catch_amalgamated.hpp"

#include "core/time.hpp"

TEST_CASE("Roundtrip time") {
    const LocalTime actual;
    const auto      micros = actual.MicrosSinceMidnight();
    const LocalTime regenerated{micros};

    REQUIRE(actual.Hour == regenerated.Hour);
    REQUIRE(actual.Minute == regenerated.Minute);
    REQUIRE(actual.Second == regenerated.Second);
    REQUIRE(actual.Millisecond == regenerated.Millisecond);
    REQUIRE(actual.Microsecond == regenerated.Microsecond);
}
