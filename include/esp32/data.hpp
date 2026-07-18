#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/time.hpp"
#include <nlohmann/json.hpp>

#define MBR_JSON "MBR_data.json"

namespace mbr {

namespace pages { class view_page; } // namespace pages

struct app_context;

using json = nlohmann::json;

class telemetry_data {
  public:
    struct packed_data {
        std::vector<uint64_t>                                time_micros_raw;
        std::vector<double>                                  time_minutes_normalized;
        std::unordered_map<std::string, std::vector<double>> series;
        std::vector<std::string>                             raw_lines;
    };

    struct data_info {
        std::string key;
        std::string name;
        bool        required = false;
        bool        plot     = false;
        std::string unit;
        std::string group;
    };

  public:
    explicit telemetry_data() {
        init_json();
        init_data();
    }
    ~telemetry_data() = default;

    std::vector<data_info>                               data_values;
    std::unordered_map<std::string, std::vector<double>> series;
    [[nodiscard]] const std::vector<double>&             get_time() const { return time_; }
    [[nodiscard]] const std::vector<uint64_t>&           get_time_no_normal() const {
        return time_no_normal_micros_;
    }

    [[nodiscard]] const std::vector<std::string>&  get_raw_lines() const { return raw_lines_; };
    [[nodiscard]] const std::optional<local_time>& get_sync_lt() const { return sync_lt_; }
    [[nodiscard]] const std::string&               get_current_line() { return current_line_; }

    void write_data(const std::string& identifier, const std::string& value);
    void write_raw_line(const std::string& message);
    void save_current_line(const std::string& line);
    void clear();

  private:
    void init_json();
    void init_data();

  private:
    std::string               current_line_;
    std::vector<uint64_t>     time_no_normal_micros_;
    std::vector<double>       time_;
    std::vector<std::string>  raw_lines_;
    double                    sync_start_;
    std::optional<local_time> sync_lt_;

    friend class pages::view_page;
};

} // namespace mbr
