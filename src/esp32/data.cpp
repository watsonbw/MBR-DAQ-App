#include <exception>
#include <fstream>

#include "esp32/data.hpp"


#include "core/log.hpp"
#include "core/time.hpp"


bool TelemetryData::InitJSON(std::vector<DataConfig>& out) const {
    std::ifstream f("MBR_data.json");
    if (!f.is_open()){
        LOG_ERROR("JSON FILE NOT OPENED");
        return false;
    }
    DataConfig temp;
    json jason = json::parse(f);
    try {
        for (const auto &data : jason.at("fields")) {
            temp.Key = data.at("key").get<std::string>();
            temp.Name = data.at("name").get<std::string>();
            temp.Required = data.at("required").get<bool>();
            temp.Plot = data.at("plot").get<bool>();
            temp.Unit = data.value("unit", "");
            temp.Group = data.value("group", "");
            out.push_back(temp);
        }
    } catch (const json::exception& e) {
            LOG_ERROR("Invalid JSON config: {}", e.what());
            return false;
    }
    return true;
}



void TelemetryData::WriteData(const std::string &identifier, const std::string &value){
try {
    double val = std::stod(value);
    m_Series[identifier].push_back(val);
} catch (const std::exception& e){
    LOG_ERROR("Couldn't write to existng data vector");
}
}
