#include "esp32/data.hpp"

#include <exception>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "core/log.hpp"
#include <stdx/profiler.hh>

namespace mbr {

telemetry_data::telemetry_data(log_fn_t log) : log_{std::move(log)} {
    init_json();
    init_data();
}

void telemetry_data::init_json() {
    std::ifstream f{DEFAULT_JSON_PATH};
    if (!f.is_open()) {
        log_error(log_, "JSON FILE NOT OPENED");
        return;
    }
    data_info temp;
    json      jason = json::parse(f);
    try {
        for (const auto& data : jason.at("fields")) {
            temp.key      = data.at("key").get<std::string>();
            temp.name     = data.at("name").get<std::string>();
            temp.required = data.at("required").get<bool>();
            temp.plot     = data.at("plot").get<bool>();
            temp.unit     = data.value("unit", "");
            temp.group    = data.value("group", "");
            data_values.emplace_back(std::move(temp));
        }
    } catch (const json::exception& e) {
        log_error(log_, "Invalid JSON config: {}", e.what());
        return;
    }
}

void telemetry_data::init_data() {
    for (const auto& data : data_values) { series.try_emplace(data.key, std::vector<f64>{}); }
}

void telemetry_data::write_data(const std::string& identifier, const std::string& value) {
    PROFILE_FUNCTION();
    try {
        f64 val = std::stod(value);
        series[identifier].emplace_back(val);
    } catch (const std::exception& e) { log_error(log_, "Couldn't write to existng data vector"); }
}

void telemetry_data::clear() {
    for (auto& [key, values] : series) { values.clear(); }
    time_.clear();
    time_no_normal_micros_.clear();
    raw_lines_.clear();
    current_line_.clear();
    sync_lt_.reset();
}

void telemetry_data::save_current_line(const std::string& line) { current_line_ = line; }

void telemetry_data::write_raw_line(const std::string& full_message) {
    raw_lines_.emplace_back(full_message);
}

} // namespace mbr
