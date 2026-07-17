#include <exception>
#include <fstream>

#include "esp32/data.hpp"

#include "core/log.hpp"

void TelemetryData::InitJSON() {
    std::ifstream f(MBR_JSON);
    if (!f.is_open()) {
        LOG_ERROR("JSON FILE NOT OPENED");
        return;
    }
    DataInfo temp;
    json     jason = json::parse(f);
    try {
        for (const auto& data : jason.at("fields")) {
            temp.Key      = data.at("key").get<std::string>();
            temp.Name     = data.at("name").get<std::string>();
            temp.Required = data.at("required").get<bool>();
            temp.Plot     = data.at("plot").get<bool>();
            temp.Unit     = data.value("unit", "");
            temp.Group    = data.value("group", "");
            DataValues.push_back(temp);
        }
    } catch (const json::exception& e) {
        LOG_ERROR("Invalid JSON config: {}", e.what());
        return;
    }
}

void TelemetryData::InitData() {
    for (const auto& data : DataValues) {
        Series.try_emplace(data.Key, std::vector<double>{});
    }
}

void TelemetryData::WriteData(const std::string& identifier, const std::string& value) {
    try {
        double val = std::stod(value);
        Series[identifier].push_back(val);
    } catch (const std::exception& e) { LOG_ERROR("Couldn't write to existng data vector"); }
}

void TelemetryData::Clear() {
    for (auto& [key, values] : Series) {
        values.clear();
    }
    m_Time.clear();
    m_TimeNoNormalMicros.clear();
    m_RawLines.clear();
    m_CurrentLine.clear();
    m_SyncLT.reset();
}

void TelemetryData::SaveCurrentLine(const std::string& line) { m_CurrentLine = line; }

void TelemetryData::WriteRawLine(const std::string& full_message) {
    m_RawLines.push_back(full_message);
}
