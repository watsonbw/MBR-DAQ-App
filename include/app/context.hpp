#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <QWidget>

#include <stdx/types.hh>

#include "core/log.hpp"
#include "esp32/backend.hpp"

namespace mbr {

enum class page_type_t : u8 {
    HOME,
    PLOT,
    ANALYSIS,
    SERIAL,
    SETTINGS,
};

[[nodiscard]] constexpr const char* page_type_str(page_type_t page_type) {
    switch (page_type) {
    case page_type_t::HOME:   return "Home";
    case page_type_t::PLOT:    return "Plot";
    case page_type_t::ANALYSIS:  return "Analysis";
    case page_type_t::SERIAL: return "Serial Monitor";
    case page_type_t::SETTINGS: return "Settings";
    default:                  return "Unknown";
    }
}

class telemetry_backend;

struct app_context {
    log_t             logger;
    page_type_t       current_page_type;
    std::atomic<bool> should_exit{false};
    bool              is_cmd_input_focused{false};

    std::unique_ptr<telemetry_backend> backend;
    std::string                        username;
    std::string                        password;
    log_fn_t                           log;
};

} // namespace mbr
