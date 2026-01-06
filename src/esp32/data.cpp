#include <exception>

#include "esp32/data.hpp"

#include "core/log.hpp"
#include "core/time.hpp"

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

TelemetryData::TelemetryData() {
    m_RPMData.Reserve();
    m_ShockData.Reserve();
}

void TelemetryData::WriteData(const std::string& identifier, const std::string& value) {
    try {
        if (identifier == "T") {
            LocalTime lt{std::stoull(value)};
            m_TimeNoNormalMicros.push_back(lt.MicrosSinceMidnight());

            const auto minutes_from_mid = lt.MinutesSinceMidnight();
            if (!m_SyncLT.has_value()) {
                m_SyncLT    = lt;
                m_SyncStart = minutes_from_mid;
            }
            m_Time.push_back(minutes_from_mid - m_SyncStart);
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
    } catch (std::exception e) { LOG_ERROR("Failed to write data: {}", e.what()); }
}

void TelemetryData::WriteRawLine(const std::string& full_message) {
    m_RawLines.push_back(full_message);
}

void TelemetryData::Clear() {
    m_RawLines.clear();
    m_RPMData.Clear();
    m_ShockData.Clear();
    m_Time.clear();
    m_TimeNoNormalMicros.clear();
    m_SyncLT = std::nullopt;
}
