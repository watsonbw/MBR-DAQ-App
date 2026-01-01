#pragma once

#include <atomic>
#include <mutex>

#include "app/style.hpp"

#include "esp32/data.hpp"

enum PageType {
    HOME,
    RPM,
    SHOCK,
    VIEW,
};

const char* PageTypeString(PageType type);

struct AppContext {
    AppFonts Fonts;
    PageType CurrentPageType;

    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};
    std::atomic<bool> ShouldExit{false};

    std::mutex    DataMutex;
    TelemetryData Data;
    std::string   Username;
    std::string   Password;
};
