#include "esp32/data.hpp"

#include "app/context.hpp"

void RPMData::Reserve(size_t size) {
    EngineRPM.reserve(size);
    WheelRPM.reserve(size);
}

void RPMData::Clear() {
    EngineRPM.clear();
    WheelRPM.clear();
}

void ShockData::Reserve(size_t size) {
    FrontRight.reserve(size);
    FrontLeft.reserve(size);
    BackRight.reserve(size);
    BackLeft.reserve(size);
}

void ShockData::Clear() {
    FrontRight.clear();
    FrontLeft.clear();
    BackRight.clear();
    BackLeft.clear();
}

TelemetryData::TelemetryData(std::shared_ptr<AppContext> ctx) : m_Context{ctx}, m_Time{0.0f} {
    m_RPMData.Reserve();
    m_ShockData.Reserve();
}

TelemetryData::~TelemetryData() {
    if (m_Worker.joinable()) { m_Worker.join(); }
}

void TelemetryData::Start() { m_Worker = std::thread(&TelemetryData::WorkerLoop, this); }

void TelemetryData::WorkerLoop() {
    while (!m_Context->ShouldExit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void TelemetryData::PopulateData([[maybe_unused]] const char* esp32_data) {
    // Empty the data buffers but retain their capacity
    m_RPMData.Clear();
    m_ShockData.Clear();

    // Do something fun with the raw bytes to parse
}
