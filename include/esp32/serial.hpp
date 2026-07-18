#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

#include <serial/serial.h>

#include "core/log.hpp"

namespace mbr {

class serial_manager_t {
  public:
    explicit serial_manager_t(int baud_rate = 115'200, int timeout_ms = 0, log_fn_t log = nullptr)
        : log_{std::move(log)}, baud_rate_(baud_rate), timeout_ms_(timeout_ms) {}
    ~serial_manager_t();

    serial_manager_t(const serial_manager_t&)            = delete;
    serial_manager_t& operator=(const serial_manager_t&) = delete;
    serial_manager_t(serial_manager_t&&)                 = delete;
    serial_manager_t& operator=(serial_manager_t&&)      = delete;

    std::atomic<bool> keep_running{true};
    std::atomic<bool> should_send_data{false};
    std::atomic<bool> is_serial_write{false};

    [[nodiscard]] std::vector<std::string>        get_all_ports() const;
    [[nodiscard]] std::unordered_set<std::string> get_chosen_ports() const;
    [[nodiscard]] std::vector<std::string>        return_data_stream() const;

    void     send_data(const std::string& msg);
    void     receive_data();
    void     read_all();
    void     start();
    void     add_port(const std::string& port);
    void     remove_port(const std::string& port);
    void     clean_ports();
    void     close_port(const std::string& port);
    void     close_all();
    bool     open_port(const std::string& port, const std::string& description = "");
    bool     is_port_selected(const std::string& port);
    void     change_baud_rate(uint32_t baud);
    void     stop();
    bool     is_running() const;
    uint32_t get_baud_rate() { return baud_rate_; }

  private:
    log_fn_t                                                         log_;
    int                                                              baud_rate_;
    int                                                              timeout_ms_;
    std::unordered_map<std::string, std::unique_ptr<serial::Serial>> ports_;
    std::unordered_set<std::string>                                  chosen_ports_;
    std::thread                                                      worker_;
    std::vector<std::string>                                         input_stream_;
    mutable std::recursive_mutex                                     mutex_;
};

} // namespace mbr
