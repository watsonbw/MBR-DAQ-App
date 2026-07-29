#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <nlohmann/json.hpp>
#include <stdx/option.hh>
#include <stdx/type_traits.hh>
#include <stdx/types.hh>

#include "core/log.hpp"
#include "core/time.hpp"

namespace mbr {

namespace pages { class view_page; } // namespace pages

struct app_context;

using json = nlohmann::json;

class telemetry_data {
  public:
    struct data_info {
        std::string key;
        std::string name;
        bool        required = false;
        bool        plot     = false;
        std::string unit;
        std::string group;
    };

  public:
    explicit telemetry_data(const std::filesystem::path& json_path, log_fn_t log = nullptr);

    std::vector<data_info>                data_values;
    [[nodiscard]] const std::vector<f64>& get_time() const { return time_; }
    [[nodiscard]] const std::vector<u64>& get_time_no_normal() const {
        return time_no_normal_micros_;
    }

    [[nodiscard]] const std::vector<std::string>& get_raw_lines() const { return raw_lines_; };
    [[nodiscard]] const stdx::option<local_time>& get_sync_lt() const { return sync_lt_; }
    [[nodiscard]] const std::string&              get_current_line() { return current_line_; }

    template <typename Self>
    [[nodiscard]] auto& get_series(this Self&& self, const std::string& key) noexcept {
        auto it = self.series_.find(key);
        if (it != self.series_.end()) { return it->second; }

        if (!self.logged_missing_keys_.contains(key)) {
            self.logged_missing_keys_.insert(key);
            log_error(self.log_, "Key '{}' not found in telemetry configuration!", key);
        }
        static stdx::const_dispatch_t<Self, std::vector<f64>> dummy;
        return dummy;
    }

    [[nodiscard]] auto& get_series() noexcept { return series_; }

    void set_sync_lt(local_time lt) noexcept { sync_lt_ = lt; }
    void write_data(const std::string& identifier, const std::string& value);
    void write_raw_line(const std::string& message);
    void save_current_line(const std::string& line);
    void clear();

  private:
    void init_json(const std::filesystem::path& path);
    void init_data();

  private:
    log_fn_t                                                       log_;
    std::string                                                    current_line_;
    std::vector<u64>                                               time_no_normal_micros_;
    std::vector<double>                                            time_;
    std::vector<std::string>                                       raw_lines_;
    double                                                         sync_start_;
    stdx::option<local_time>                                       sync_lt_;
    ankerl::unordered_dense::map<std::string, std::vector<double>> series_;
    mutable ankerl::unordered_dense::set<std::string>              logged_missing_keys_;

    friend class pages::view_page;
};

} // namespace mbr
