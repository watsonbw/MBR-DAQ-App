#include <string>

#include <catch_amalgamated.hpp>

#include "core/log.hpp"
#include "core/time.hpp"
#include "esp32/data.hpp"
#include "helpers/backend_json_fixture.hpp"

namespace mbr::tests {

TEST_CASE("telemetry_data config loading and data writing") {
    helpers::backend_json_fixture fixture;
    log_t                         logger;
    telemetry_data                data{fixture.temp.path, logger.get_log_fn()};

    SECTION("Configuration was successfully loaded from JSON") {
        REQUIRE(data.data_values.size() == 3);
        CHECK(data.data_values[0].key == "T");
        CHECK(data.data_values[0].name == "Timestamp");
        CHECK(data.data_values[0].required);
        CHECK_FALSE(data.data_values[0].plot);

        CHECK(data.data_values[1].key == "E");
        CHECK(data.data_values[1].unit == "RPM");
        CHECK(data.data_values[1].group == "rpm");
    }

    SECTION("Writing and parsing values") {
        data.write_data("T", "60000000"); // 60,000,000 micros = 1 minute
        data.write_data("E", "3500.5");

        const auto& times     = data.get_time();
        const auto& times_raw = data.get_time_no_normal();

        REQUIRE(times.size() == 1);
        REQUIRE(times_raw.size() == 1);
        CHECK(times_raw[0] == 60'000'000ULL);
        CHECK(times[0] == Catch::Approx(1.0));

        const auto& series_t = data.get_series("T");
        const auto& series_e = data.get_series("E");

        REQUIRE(series_t.get()->size() == 1);
        CHECK(series_t.get()->at(0) == Catch::Approx(60'000'000.0));
        REQUIRE(series_e.get()->size() == 1);
        CHECK(series_e.get()->at(0) == Catch::Approx(3500.5));
    }

    SECTION("Handling missing keys") {
        const auto& series_missing = data.get_series("UNKNOWN_KEY");
        CHECK(series_missing.get()->empty());
        std::string logs = logger.get_streamed_logs();
        CHECK(logs.contains("Key 'UNKNOWN_KEY' not found in telemetry configuration!"));
    }

    SECTION("Clearing data resets all states") {
        data.write_data("T", "1000000");
        data.write_data("E", "1200");
        data.write_raw_line("Raw message line");
        data.save_current_line("COM3: T 1000000 E 1200");
        data.set_sync_lt(local_time{12, 0, 0, 0, 0});

        CHECK_FALSE(data.get_time().empty());
        CHECK_FALSE(data.get_raw_lines().empty());
        CHECK(data.get_sync_lt().has_value());

        data.clear();

        CHECK(data.get_time().empty());
        CHECK(data.get_time_no_normal().empty());
        CHECK(data.get_raw_lines().empty());
        CHECK_FALSE(data.get_sync_lt().has_value());
    }
}

} // namespace mbr::tests
