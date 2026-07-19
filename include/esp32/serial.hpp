#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <ankerl/unordered_dense.h>
#include <serial/serial.h>
#include <stdx/types.hh>

#include "core/log.hpp"
#include "stdx/hash.hh"

namespace mbr {

class serial_manager_t {
  public:
    using port_map_t        = ankerl::unordered_dense::map<std::string,
                                                           std::unique_ptr<serial::Serial>,
                                                           stdx::string_transparent_hash,
                                                           stdx::string_transparent_eq>;
    using chosen_port_set_t = ankerl::unordered_dense::
        set<std::string, stdx::string_transparent_hash, stdx::string_transparent_eq>;

  public:
    explicit serial_manager_t(i32 baud_rate = 115'200, i32 timeout_ms = 0, log_fn_t log = nullptr)
        : log_{std::move(log)}, baud_rate_(baud_rate), timeout_ms_(timeout_ms) {}
    ~serial_manager_t();

    serial_manager_t(const serial_manager_t&)            = delete;
    serial_manager_t& operator=(const serial_manager_t&) = delete;
    serial_manager_t(serial_manager_t&&)                 = delete;
    serial_manager_t& operator=(serial_manager_t&&)      = delete;

    std::atomic<bool> keep_running{true};
    std::atomic<bool> should_send_data{false};
    std::atomic<bool> is_serial_write{false};

    [[nodiscard]] std::vector<std::string> get_all_ports() const;
    [[nodiscard]] chosen_port_set_t        get_chosen_ports() const;
    [[nodiscard]] std::vector<std::string> return_data_stream() const;

    void send_data(const std::string& msg);
    void receive_data();
    void read_all();
    void start();
    void add_port(const std::string& port);
    void remove_port(const std::string& port);
    void clean_ports();
    void close_port(const std::string& port);
    void close_all();
    bool open_port(const std::string& port, const std::string& description = "");
    bool is_port_selected(const std::string& port);
    void change_baud_rate(u32 baud);
    void stop();
    bool is_running() const;
    u32  get_baud_rate() const { return baud_rate_; }

  private:
    log_fn_t                     log_;
    i32                          baud_rate_;
    i32                          timeout_ms_;
    port_map_t                   ports_;
    chosen_port_set_t            chosen_ports_;
    std::jthread                 worker_;
    std::vector<std::string>     input_stream_;
    mutable std::recursive_mutex mutex_;
};

} // namespace mbr
