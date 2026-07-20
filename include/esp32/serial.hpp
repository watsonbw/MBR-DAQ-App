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
#include <stdx/hash.hh>
#include <stdx/types.hh>
#include <stdx/utility.hh>

#include "core/log.hpp"

namespace mbr {

enum class baud_rate_t : u8 {
    THREE,
    TWELVE,
    TWENTYFOUR,
    FORTYEIGHT,
    NINETYSIX,
    ONENIGHTYTWO,
    THREEEIGHTYFOUR,
    FIVESEVENTYSIX,
    ONEONEFIFTYTWO,
};

[[nodiscard]] u32         baud_rate_value(baud_rate_t baud) noexcept;
[[nodiscard]] const char* baud_rate_string(baud_rate_t baud) noexcept;

class serial_manager_t {
  public:
    using port_map_t        = ankerl::unordered_dense::map<std::string,
                                                           std::unique_ptr<serial::Serial>,
                                                           stdx::string_transparent_hash,
                                                           stdx::string_transparent_eq>;
    using chosen_port_set_t = ankerl::unordered_dense::
        set<std::string, stdx::string_transparent_hash, stdx::string_transparent_eq>;

  public:
    explicit serial_manager_t(baud_rate_t baud_rate  = baud_rate_t::ONEONEFIFTYTWO,
                              i32         timeout_ms = 0,
                              log_fn_t    log        = nullptr)
        : log_{std::move(log)}, timeout_ms_{timeout_ms}, baud_rate_{baud_rate} {}
    ~serial_manager_t();
    MAKE_PINNED(serial_manager_t);

    [[nodiscard]] bool should_send_data() const noexcept {
        return should_send_data_.load(std::memory_order_relaxed);
    }
    void set_send_data(bool value) noexcept {
        should_send_data_.store(value, std::memory_order_relaxed);
    }

    [[nodiscard]] bool is_serial_write() const noexcept {
        return is_serial_write_.load(std::memory_order_relaxed);
    }
    void set_serial_write(bool value) noexcept {
        is_serial_write_.store(value, std::memory_order_relaxed);
    }

    [[nodiscard]] std::recursive_mutex&           get_mutex() const noexcept { return mutex_; }
    [[nodiscard]] const std::vector<std::string>& get_data_stream() const noexcept {
        return input_stream_;
    }

    [[nodiscard]] std::vector<std::string> get_all_ports() const;
    [[nodiscard]] chosen_port_set_t        get_chosen_ports() const;

    void        send_data(const std::string& msg);
    void        receive_data();
    void        read_all();
    void        start();
    void        add_port(const std::string& port);
    void        remove_port(const std::string& port);
    void        clean_ports();
    void        close_port(const std::string& port);
    void        close_all();
    bool        open_port(const std::string& port, const std::string& description = "");
    bool        is_port_selected(const std::string& port);
    void        change_baud_rate(baud_rate_t baud);
    void        stop();
    bool        is_running() const;
    baud_rate_t get_baud_rate() const { return baud_rate_; }

  private:
    log_fn_t                     log_;
    i32                          timeout_ms_;
    baud_rate_t                  baud_rate_;
    std::atomic<bool>            keep_running_{true};
    std::atomic<bool>            should_send_data_{false};
    std::atomic<bool>            is_serial_write_{false};
    port_map_t                   ports_;
    chosen_port_set_t            chosen_ports_;
    std::thread                  worker_;
    std::vector<std::string>     input_stream_;
    mutable std::recursive_mutex mutex_;
};

} // namespace mbr
