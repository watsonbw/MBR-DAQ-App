#include <filesystem>
#include <fstream>

#include <catch_amalgamated.hpp>

#include "core/log.hpp"
#include "esp32/backend.hpp"

namespace mbr::tests {

struct backend_json_fixture {
    backend_json_fixture() {
        std::ofstream f{"MBR_data.json"};
        f << R"({
          "fields": [
            { "key": "T",  "name": "Timestamp",   "required": true,  "plot": false },
            { "key": "E",  "name": "Engine RPM",  "required": false, "plot": true, "unit": "RPM",  "group": "rpm" },
            { "key": "W",  "name": "Wheel RPM",   "required": false, "plot": true, "unit": "RPM",  "group": "rpm" }
          ]
        })";
    }

    ~backend_json_fixture() { std::filesystem::remove("MBR_data.json"); }
};

TEST_CASE("telemetry_backend packet validation") {
    backend_json_fixture fixture;
    log_t                logger;
    telemetry_backend    backend{logger.get_log_fn()};

    SECTION("validate_packet with valid packets") {
        auto result = backend.validate_packet("T 123456 E 3000 W 1500");
        REQUIRE(result.has_value());
        REQUIRE(result->size() == 3);
        CHECK((*result)[0].first == "T");
        CHECK((*result)[0].second == "123456");
        CHECK((*result)[1].first == "E");
        CHECK((*result)[1].second == "3000");
    }

    SECTION("validate_packet with missing timestamp") {
        auto result = backend.validate_packet("E 3000 W 1500");
        CHECK_FALSE(result.has_value());
    }

    SECTION("validate_packet with invalid keys") {
        auto result = backend.validate_packet("T 123456 INVALID_KEY 3000");
        CHECK_FALSE(result.has_value());
    }

    SECTION("validate_packet with non-numeric timestamp") {
        auto result = backend.validate_packet("T abc E 3000");
        CHECK_FALSE(result.has_value());
    }

    SECTION("validate_packet with extra whitespace") {
        auto result = backend.validate_packet("  T   123456    E   3000  ");
        REQUIRE(result.has_value());
        REQUIRE(result->size() == 2);
        CHECK((*result)[0].first == "T");
        CHECK((*result)[0].second == "123456");
    }
}

} // namespace mbr::tests
