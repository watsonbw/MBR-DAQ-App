#include <catch_amalgamated.hpp>

#include "core/log.hpp"

namespace mbr::tests {

TEST_CASE("log_t initialization and routing") {
    log_t    logger;
    log_fn_t log_fn = logger.get_log_fn();
    REQUIRE(log_fn);

    SECTION("Logs get captured in ostream") {
        log_info(log_fn, "Hello, {}!", "World");
        const auto logs = logger.get_streamed_logs();
        CHECK(logs.contains("[info]: Hello, World!"));
    }

    SECTION("Logging macros format correctly") {
        log_trace(log_fn, "Trace test");
        log_warn(log_fn, "Warning number {}", 67);
        log_error(log_fn, "Error occurred");
        log_critical(log_fn, "Critical alert: {}", "glorp");

        const auto logs = logger.get_streamed_logs();
#ifdef LOGGING
        CHECK(logs.contains("[trace]: Trace test"));
#endif
        CHECK(logs.contains("[warning]: Warning number 67"));
        CHECK(logs.contains("[error]: Error occurred"));
        CHECK(logs.contains("[critical]: Critical alert: glorp"));
    }

    SECTION("Null log function does not crash") {
        log_fn_t null_log = nullptr;
        log_trace(null_log, "No-op trace");
        log_info(null_log, "No-op info");
        log_warn(null_log, "No-op warn");
        log_error(null_log, "No-op error");
        log_critical(null_log, "No-op critical");
    }
}

} // namespace mbr::tests
