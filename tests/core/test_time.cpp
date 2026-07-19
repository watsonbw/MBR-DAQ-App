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

} // namespace mbr::tests
