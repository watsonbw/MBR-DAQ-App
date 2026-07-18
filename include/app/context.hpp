#pragma once

#include <atomic>
#include <cstdint>
#include <memory>

#include "app/style.hpp"
#include "esp32/backend.hpp"

namespace mbr {

enum class page_type_t : uint8_t {
    HOME,
    RPM,
    SHOCK,
    VIEW,
    SERIAL,
};

[[nodiscard]] constexpr const char* page_type_str(page_type_t page_type) {
    switch (page_type) {
    case page_type_t::HOME:   return "Home";
    case page_type_t::RPM:    return "RPM";
    case page_type_t::SHOCK:  return "Shock";
    case page_type_t::VIEW:   return "View";
    case page_type_t::SERIAL: return "Serial Monitor";
    default:                  return "Unknown";
    }
}

class TelemetryBackend;

struct app_context {
    app_style         style;
    page_type_t       current_page_type;
    std::atomic<bool> should_exit{false};
    bool              is_cmd_input_focused{false};

    std::unique_ptr<TelemetryBackend> backend;
    std::string                       username;
    std::string                       password;
};

} // namespace mbr
