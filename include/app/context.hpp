#pragma once

#include <atomic>
#include <mutex>

#include "esp32/data.hpp"

struct AppContext {
    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};
    std::atomic<bool> ShouldExit{false};

    std::mutex    DataMutex;
    TelemetryData Data;
};
