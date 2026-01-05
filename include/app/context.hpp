#pragma once

#include <atomic>
#include <memory>

#include "app/style.hpp"

#include "esp32/backend.hpp"

enum PageType {
    HOME,
    RPM,
    SHOCK,
    VIEW,
};

const char* PageTypeString(PageType type);

class TelemetryBackend;

struct AppContext {
    AppStyle                          Style;
    PageType                          CurrentPageType;
    std::atomic<bool>                 ShouldExit{false};
    std::unique_ptr<TelemetryBackend> Backend;

    std::string Username;
    std::string Password;
};
