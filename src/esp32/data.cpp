#include "esp32/data.hpp"

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

TelemetryData::TelemetryData() : m_Time{0.0f} {
    m_RPMData.Reserve();
    m_ShockData.Reserve();
}

void TelemetryData::WriteData([[maybe_unused]] std::string identifier,
                              [[maybe_unused]] std::string value) {}
