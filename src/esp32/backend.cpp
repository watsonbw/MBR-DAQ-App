#include "esp32/backend.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <chrono>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <system_error>
#include <thread>
#include <type_traits>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>
#include <ixwebsocket/IXWebSocketMessage.h>
#include <ixwebsocket/IXWebSocketMessageType.h>
#include <ixwebsocket/IXWebSocketSendInfo.h>
#include <stdx/assert.hh>
#include <stdx/fixed/hash_table.hh>
#include <stdx/hash.hh>
#include <stdx/option.hh>
#include <stdx/profiler.hh>
#include <stdx/string.hh>
#include <stdx/types.hh>

#include "app/context.hpp"
#include "core/ip.hpp"
#include "core/log.hpp"
#include "core/time.hpp"
#include "esp32/data.hpp"
#include "esp32/serial.hpp"

using namespace std::chrono_literals;

namespace mbr {

telemetry_backend::telemetry_backend(log_fn_t log)
    : log_{std::move(log)}, data_{log_}, serial_manager_{baud_rate_t::ONEONEFIFTYTWO, 500, log_} {
    buffer_.reserve(4'096);
    register_handlers();
}

telemetry_backend::~telemetry_backend() { kill(); }

void telemetry_backend::start() {
    log_info(log_, "Started Connection Attempt");
    kill();
    should_kill_ = false;

    ASSERT(ip_addr_.is_valid(), "Telemetry backend initialized with invalid ip address");
    const auto real_addr = fmt::format("ws://{}/ws", ip_addr_.to_string());
    log_info(log_, "Attempting to connect with address: {}", real_addr);
    web_sockets_.setUrl(real_addr);

    web_sockets_.setMaxWaitBetweenReconnectionRetries(5'000);
    web_sockets_.setMinWaitBetweenReconnectionRetries(1'000);
    web_sockets_.enableAutomaticReconnection();
    // m_WebSocket.setPingInterval(30);
    web_sockets_.setHandshakeTimeout(10);

    web_sockets_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            is_connected_ = true;
            is_receiving_ = false;
            send_cmd("STATUS");
            log_info(log_, "Connected to ESP32");
        }

        if (msg->type == ix::WebSocketMessageType::Close ||
            msg->type == ix::WebSocketMessageType::Error) {
            is_connected_ = false;
            is_receiving_ = false;
        }
        if (msg->type == ix::WebSocketMessageType::Close) { log_warn(log_, "WebSocket closed"); }

        if (msg->type == ix::WebSocketMessageType::Error) {
            log_error(log_, "WebSocket error: {}", msg->errorInfo.reason.c_str());
            log_error(log_, "HTTP Status: {}", msg->errorInfo.http_status);
        }

        if (msg->type == ix::WebSocketMessageType::Message) {
            is_receiving_ = true;
            this->on_message(msg);
            last_data_time_ = std::chrono::steady_clock::now();
        }
    });

    web_sockets_.start();
    worker_ = std::jthread(&telemetry_backend::worker_loop, this);
}

void telemetry_backend::kill() {
    web_sockets_.stop();
    if (worker_.joinable()) {
        should_kill_ = true;
        worker_.join();
    }
}

void telemetry_backend::send_cmd(const std::string& text) {
    if (web_sockets_.getReadyState() == ix::ReadyState::Open) {
        const ix::WebSocketSendInfo info = web_sockets_.send(text);
        if (!info.success) {
            log_warn(log_, "Send failed:");
            is_connected_ = false;
            return;
        }

        log_info(log_, "Sent command: {}", text);
        return;
    }

    log_warn(log_, "Failed to send message as WebSocket was not open");
}

void telemetry_backend::worker_loop() {
    while (!should_kill_) {
        std::this_thread::sleep_for(10ms);
        if (web_sockets_.getReadyState() != ix::ReadyState::Open) { is_connected_ = false; }
        if (std::chrono::steady_clock::now() - last_data_time_.load() > 500ms) {
            is_receiving_ = false;
        }
    }
}

void telemetry_backend::on_message(const ix::WebSocketMessagePtr& msg) {
    PROFILE_FUNCTION();
    if (msg->type == ix::WebSocketMessageType::Message) {
        buffer_.append(msg->str);
        usize newline_pos;
        while ((newline_pos = buffer_.find('\n')) != std::string::npos) {
            const auto line = buffer_.substr(0, newline_pos);
            buffer_.erase(0, newline_pos + 1);

            // check for commands/responses first
            if (line.starts_with("RES")) {
                handle_response(line);
                continue;
            }

            const auto parsed = validate_packet(line);
            if (!parsed) { continue; }

            // Now we can safely unpack the packet
            const std::unique_lock lock{data_latch_};

            // write data
            if (!is_logging_) { continue; }

            for (const auto& [ident, value] : parsed.value()) {
                data_.write_data(std::string{ident}, std::string{value});
            }
            data_.write_raw_line(line);
            for (const auto& [ident, value] : parsed.value()) {
                if (ident == "W") {
                    data_.save_current_line(line);
                    break;
                }
            }
        }
    }
}

// This function serves to handle the incoming message recovered from OnMessage.
// The goal is to primarily handle the data packing itself and doesn't have any command
// or response handling. See "HandleResponse" for response logic
stdx::option<std::vector<std::pair<std::string_view, std::string_view>>>
telemetry_backend::validate_packet(std::string_view str) const {
    PROFILE_FUNCTION();
    std::vector<std::pair<std::string_view, std::string_view>> parsed;
    parsed.reserve(data_.data_values.size());

    // runs through the whole sent packet. this ensures that the packet must be valid but doesn't
    // need every m_packetfield.
    std::string_view current = str;
    while (!current.empty()) {
        current = stdx::string::trim_left(current);
        if (current.empty()) { break; }

        // Find space between key and value
        const usize key_end = current.find(' ');
        if (key_end == std::string_view::npos) { return stdx::none; }
        const auto key = stdx::string::substr(current, 0, key_end);

        // Get value portion after the key
        auto value_part = stdx::string::trim_left(stdx::string::substr(current, key_end + 1));
        if (value_part.empty()) { return stdx::none; }

        // Find the end of the value (either next space or EOL)
        const usize value_end = value_part.find(' ');
        const auto  value     = stdx::string::substr(value_part, 0, value_end);

        // Advance current past the parsed key-value pair
        current = value_end == std::string_view::npos
                      ? std::string_view{}
                      : stdx::string::substr(value_part, value_end + 1);

        // Validate the key is actually allowed
        const auto valid_key =
            std::ranges::any_of(data_.data_values, [&](const auto& dv) { return dv.key == key; });
        if (!valid_key) { return stdx::none; }

        // Validate timestamp field "T" is numeric
        if (key == "T") {
            u64  timestamp;
            auto res = std::from_chars(value.begin(), value.end(), timestamp);
            if (res.ec != std::errc{} || res.ptr != value.end()) { return stdx::none; }
        }
        parsed.emplace_back(key, value);
    }

    // Verify timestamp was parsed
    const bool has_t =
        std::ranges::any_of(parsed, [](const auto& pair) { return pair.first == "T"; });
    if (!has_t) { return stdx::none; }

    return parsed;
}

telemetry_data::packed_data telemetry_backend::pack_data() {
    const std::shared_lock lock{data_latch_};
    return {.time_micros_raw         = data_.get_time_no_normal(),
            .time_minutes_normalized = data_.get_time(),
            .series                  = data_.series,
            .raw_lines               = data_.get_raw_lines()};
}

void telemetry_backend::set_ip(const ipv4_t& ipv4) {
    if (!ipv4.is_valid()) {
        log_error(log_, "Requested Ip was invalid: {}", ipv4.to_string());
        return;
    }

    ip_addr_ = ipv4;
    kill();
    should_kill_ = false;
    start();
    try_connection_ = false;
}

namespace {

// You will need to update this string to enum function
// EVERY TIME you add a new command. This works in series with RegisterHandlers
// and HandleResponse
constexpr auto response_lut = [] {
    constexpr std::array mappings{std::pair{"SYNC", response_type_t::SYNC},
                                  std::pair{"SD_START", response_type_t::SDSTART},
                                  std::pair{"SD_WRITE", response_type_t::SDWRITE},
                                  std::pair{"SD_CLOSE", response_type_t::SDCLOSE}};

    stdx::fixed::hash_map<std::string_view, response_type_t, mappings.size(), stdx::crc::hash> map;
    for (const auto& op : mappings) { map.emplace(op.first, op.second); }
    return map;
}();

} // namespace

// This function is the bridge between any RES sent from our car to our app
// After checking for RES in OnMessage, this function is called to parse and act on the response
// The response is parsed by checking the string with the response type ENUM and calling the
// corresponding handler For reference, general responses look like this: RES CMD_TYPE CMD_PAYLOAD
void telemetry_backend::handle_response(std::string_view line) {
    constexpr usize res_length = 4;
    if (line.size() <= res_length) {
        log_error(log_, "Invalid response length: {}", line.size());
        return;
    }
    auto rest  = line.substr(res_length);
    auto space = rest.find(' ');
    if (space == std::string::npos) {
        log_error(log_, "No space after response type: {}", rest);
        return;
    }

    auto       command = rest.substr(0, space);
    const auto type    = response_lut.get_opt(command);
    if (!type) {
        log_error(log_, "Unknown response: {}", command);
        return;
    }

    if (auto handler = response_handlers_.get_opt(*type)) {
        (*handler)(rest.substr(space + 1));
    } else {
        log_error(log_, "No handler for response: {}", command);
    }
}

// ALL responses should be created with a lambda that takes a std::string_view and returns void
// The lambda should parse the response and log any errors or success messages
// ENUMs are created in backend.hpp so when you add a new response, add it there first
// This also requires additions to ResStringtoEnum as these are ENUM mapped lambdas,
// not just a simple string mapped lambda
void telemetry_backend::register_handlers() {
    response_handlers_[response_type_t::SYNC] = [this](std::string_view line) {
        u64 micros = 0;
        std::from_chars(line.data(), line.data() + line.size(), micros);
        local_time t{micros};
        log_info(log_, "Time successfully synced at: {}", t.to_string(false));
    };

    response_handlers_[response_type_t::SDSTART] = [this](std::string_view line) {
        if (line == "deadbeef") {
            log_error(log_, "SD Card Failed Initialization");
        } else {
            log_info(log_, "SD Card Initialized At {}", line);
            is_open_ = true;
        }
    };

    response_handlers_[response_type_t::SDWRITE] = [this](std::string_view line) {
        if (line == "1") {
            log_info(log_, "SD Card Has Begun Writing");
            is_writing_ = true;
        } else if (line == "0") {
            log_info(log_, "SD Card Has Stopped Writing");
            is_writing_ = false;
        }
    };

    response_handlers_[response_type_t::SDCLOSE] = [this](std::string_view line) {
        if (line == "1") {
            is_open_ = false;
            log_info(log_, "SD Card Has Closed Succesfully");
        } else if (line == "0") {
            log_info(log_, "SD Card Failed Close");
        }
    };
}

/*
auto TelemetryBackend::HandleCommand(
    stdx::option<std::vector<std::pair<std::string_view, std::string_view>>> parsed) -> void {
    if (!parsed || parsed->size() < 2) {
        LOG_ERROR("Command Not Found");
        return;
    }
    auto response = parsed.value()[1];
    if (response.first == "SYNC") {
        u64 micros = 0;
        std::from_chars(
            response.second.data(), response.second.data() + response.second.size(), micros);
        LocalTime t{micros};
        LOG_INFO("Time successfully synced at: {}", t.String(false));
    } else if (response.first == "SD_START") {
        if (response.second == "deadbeef") {
            LOG_ERROR("SD Card Failed Initialization");
        } else {
            LOG_INFO("SD Card Initialized At {}", response.second);
            IsOpen = true;
        }
    } else if (response.first == "SD_WRITE") {
        if (response.second == "1") {
            LOG_INFO("SD Card Has Begun Writing");
            IsWriting = true;
        } else if (response.second == "0") {
            LOG_INFO("SD Card Has Stopped Writing");
            IsWriting = false;
        }
    } else if (response.first == "SD_CLOSE") {
        if (response.second == "1") {
            IsOpen = false;
            LOG_INFO("SD Card Has Closed Succesfully");
        } else if (response.second == "0") {
            LOG_INFO("SD Card Failed Close");
        }
    } else if (parsed.value()[0].second == "1") {
        LOG_ERROR("Command Not Found");
    }
}
for (const auto& field : m_PacketFields) {
    const usize ident_start = pos;
    while (pos < str.size() && str[pos] != ' ') {
        pos += 1;
    }
    if (str.substr(ident_start, pos - ident_start) != field) { return stdx::none; }

    // There can be an arbitrary amount of spaces between idents/values
    if (pos >= str.size() || str[pos] != ' ') { return stdx::none; }
    while (pos < str.size() && str[pos] == ' ') {
        pos += 1;
    }

    const usize value_start = pos;
    while (pos < str.size() && str[pos] != ' ') {
        pos += 1;
    }
    if (value_start == pos) { return stdx::none; }
    const auto value = str.substr(value_start, pos - value_start);

    // Uncalibrated packets or unparsable times are invalid
    if (field == "T") {
        u64 t   = 0;
        auto     res = std::from_chars(value.data(), value.data() + value.size(), t);
        if (res.ec != std::errc{} || t == 0) { return stdx::none; }
    }

    parsed.emplace_back(field, value);
    while (pos < str.size() && str[pos] == ' ') {
        pos += 1;
    }
}
*/

} // namespace mbr
