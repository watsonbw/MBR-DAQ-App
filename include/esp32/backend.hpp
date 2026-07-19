#pragma once

#include <atomic>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <ixwebsocket/IXWebSocket.h>
#include <stdx/fixed/enum_map.hh>

#include "core/ip.hpp"
#include "core/log.hpp"

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

    // Returns a safely accessible grouping of relevant data.
    //
    // This automatically manages the Data's mutex!
    [[nodiscard]] telemetry_data::packed_data pack_data();
    void                                      set_ip(const ipv4_t& ipv4);

  private:
    log_fn_t log_;

  public:
    std::mutex        data_mutex;
    telemetry_data    data;
    serial_manager_t  serial_manager;
    std::atomic<bool> try_connection{false};
    std::atomic<bool> is_connected{false};
    std::atomic<bool> is_logging{false};
    std::atomic<bool> is_receiving{false};
    std::atomic<bool> is_writing{false};
    std::atomic<bool> is_open{false};

  private:
    void worker_loop();
    void on_message(const ix::WebSocketMessagePtr& msg);

    std::optional<std::vector<std::pair<std::string_view, std::string_view>>>
         validate_packet(std::string_view str) const;
    void handle_response(std::string_view line);
    void register_handlers();

  private:
    std::thread                           worker_;
    ix::WebSocket                         web_sockets_;
    std::string                           buffer_;
    ipv4_t                                ip_addr_;
    std::chrono::steady_clock::time_point last_data_time_;
    std::atomic<bool>                     should_kill_{false};

    stdx::fixed::enum_map<response_type_t, std::function<void(std::string_view)>>
        response_handlers_;
};

} // namespace mbr
