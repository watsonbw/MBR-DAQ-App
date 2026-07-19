#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <stdx/fixed/enum_map.hh>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "core/ip.hpp"
#include "core/log.hpp"
#include "core/rw_latch.hpp"
#include "esp32/data.hpp"
#include "esp32/serial.hpp"

namespace mbr {

struct app_context;

enum class response_type_t : u8 {
    SYNC,
    SDSTART,
    SDWRITE,
    SDCLOSE,
};

class telemetry_backend {
  public:
    explicit telemetry_backend(log_fn_t log = nullptr);
    ~telemetry_backend();

    void start();
    void kill();
    void send_cmd(const std::string& text);
    void set_ip(const ipv4_t& ipv4);

    [[nodiscard]] rw_latch&             get_data_latch() noexcept { return data_latch_; }
    [[nodiscard]] telemetry_data&       get_data() noexcept { return data_; }
    [[nodiscard]] const telemetry_data& get_data() const noexcept { return data_; }
    [[nodiscard]] serial_manager_t&     get_serial_manager() noexcept { return serial_manager_; }

    [[nodiscard]] std::atomic<bool>& get_try_connection() noexcept { return try_connection_; }

    [[nodiscard]] bool is_connected() const noexcept {
        return is_connected_.load(std::memory_order_relaxed);
    }
    [[nodiscard]] bool is_receiving() const noexcept {
        return is_receiving_.load(std::memory_order_relaxed);
    }
    [[nodiscard]] bool is_writing() const noexcept {
        return is_writing_.load(std::memory_order_relaxed);
    }
    [[nodiscard]] bool is_open() const noexcept { return is_open_.load(std::memory_order_relaxed); }

    [[nodiscard]] bool is_logging() const noexcept {
        return is_logging_.load(std::memory_order_relaxed);
    }
    void set_logging(bool value) noexcept { is_logging_.store(value, std::memory_order_relaxed); }

    stdx::option<std::vector<std::pair<std::string_view, std::string_view>>>
    validate_packet(std::string_view str) const;

  private:
    void worker_loop();
    void on_message(const ix::WebSocketMessagePtr& msg);

    void handle_response(std::string_view line);
    void register_handlers();

  private:
    log_fn_t                                           log_;
    rw_latch                                           data_latch_;
    telemetry_data                                     data_;
    serial_manager_t                                   serial_manager_;
    std::atomic<bool>                                  try_connection_{false};
    std::atomic<bool>                                  is_connected_{false};
    std::atomic<bool>                                  is_logging_{false};
    std::atomic<bool>                                  is_receiving_{false};
    std::atomic<bool>                                  is_writing_{false};
    std::atomic<bool>                                  is_open_{false};
    std::atomic<bool>                                  should_kill_{false};
    std::jthread                                       worker_;
    ix::WebSocket                                      web_sockets_;
    std::string                                        buffer_;
    ipv4_t                                             ip_addr_;
    std::atomic<std::chrono::steady_clock::time_point> last_data_time_;

    stdx::fixed::enum_map<response_type_t, std::function<void(std::string_view)>>
        response_handlers_;
};

} // namespace mbr
