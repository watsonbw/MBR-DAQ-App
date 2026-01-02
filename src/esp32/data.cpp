#include "esp32/data.hpp"
#include "core/log.hpp"

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

void TelemetryData::WriteData(std::string identifier, std::string value) {
    
    if (identifier == "T") {
        m_Time.push_back(std::stod(value));
    } else if (identifier == "W") {
        m_RPMData.WheelRPM.push_back(std::stod(value));
    } else if (identifier == "E") {
        m_RPMData.EngineRPM.push_back(std::stod(value));
    } else if (identifier == "fr") {
        m_ShockData.FrontRight.push_back(std::stod(value));
    } else if (identifier == "fl") {
        m_ShockData.FrontLeft.push_back(std::stod(value));
    } else if (identifier == "br") {
        m_ShockData.BackRight.push_back(std::stod(value));
    } else if (identifier == "bl") {
        m_ShockData.BackLeft.push_back(std::stod(value));
    }
}

void TelemetryData::WriteRawLine(const std::string& full_message) {
    m_RawLines.push_back(full_message);
}
