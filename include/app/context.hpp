#pragma once

#include <atomic>
#include <mutex>

#include "app/pages/page.hpp"
#include "app/style.hpp"

#include "esp32/data.hpp"

struct AppContext {
    AppFonts Fonts;
    PageType CurrentPageType;

    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};
    std::atomic<bool> ShouldExit{false};

    std::mutex    DataMutex;
    TelemetryData Data;
    std::string   username;
    std::string   password;
};
