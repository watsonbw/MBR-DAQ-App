#pragma once

#include <atomic>
#include <mutex>

#include "app/style.hpp"

#include "esp32/data.hpp"

struct AppContext {
    AppFonts Fonts;

    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};
    std::atomic<bool> ShouldExit{false};

    std::mutex    DataMutex;
    TelemetryData Data;
};
