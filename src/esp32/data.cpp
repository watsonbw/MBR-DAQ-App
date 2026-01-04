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

void TelemetryData::WriteData(std::string identifier, std::string value) {
    if (identifier == "T") {
        LocalTime lt(std::stoull(value));
        double minutes_from_mid = lt.Hour * 60.0 + static_cast<double>(lt.Minute) + lt.Second/60.0 + lt.Millisecond/60000.0;
        if (!IsSynced){
            SyncLT = lt;
            m_SyncStart = minutes_from_mid;
            IsSynced = true;
        }
        m_Time.push_back(minutes_from_mid-m_SyncStart);
        LOG_INFO("{}", minutes_from_mid);
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
    //LOG_INFO("{}{}{}", m_RPMData.WheelRPM.size(), m_RPMData.EngineRPM.size(), m_Time.size());
}

void TelemetryData::WriteRawLine(const std::string& full_message) {
    m_RawLines.push_back(full_message);
}

void TelemetryData::Clear(){
    m_RawLines.clear();
    m_RPMData.Clear();
    m_ShockData.Clear();
    m_Time.clear();
    IsSynced = false;
}
