#include <cassert>
#include <charconv>
#include <string>
#include <type_traits>

#include <fmt/format.h>
#include <ixwebsocket/IXNetSystem.h>

#include "app/context.hpp"

#include "esp32/backend.hpp"

#include "core/log.hpp"

using namespace std::chrono_literals;

namespace mbr {

telemetry_backend::telemetry_backend() : serial_manager(115'200, 500) {
    buffer_.reserve(4'096);
    register_handlers();
}

telemetry_backend::~telemetry_backend() { kill(); }

void telemetry_backend::start() {
    LOG_INFO("Started Connection Attempt");
    kill();
    should_kill_ = false;

    assert(ip_addr_.is_valid());
    const auto real_addr = fmt::format("ws://{}/ws", ip_addr_.to_string());
    LOG_INFO("Attempting to connect with address: {}", real_addr);
    web_sockets_.setUrl(real_addr);

    web_sockets_.setMaxWaitBetweenReconnectionRetries(5'000);
    web_sockets_.setMinWaitBetweenReconnectionRetries(1'000);
    web_sockets_.enableAutomaticReconnection();
    // m_WebSocket.setPingInterval(30);
    web_sockets_.setHandshakeTimeout(10);

    web_sockets_.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            is_connected = true;
            is_receiving = false;
            send_cmd("STATUS");
            LOG_INFO("Connected to ESP32");
        }

        if (msg->type == ix::WebSocketMessageType::Close ||
            msg->type == ix::WebSocketMessageType::Error) {
            is_connected = false;
            is_receiving = false;
        }
        if (msg->type == ix::WebSocketMessageType::Close) { LOG_WARN("WebSocket closed"); }

        if (msg->type == ix::WebSocketMessageType::Error) {
            LOG_ERROR("WebSocket error: {}", msg->errorInfo.reason.c_str());
            LOG_ERROR("HTTP Status: {}", msg->errorInfo.http_status);
        }

        if (msg->type == ix::WebSocketMessageType::Message) {
            is_receiving = true;
            this->on_message(msg);
            last_data_time_ = std::chrono::steady_clock::now();
        }
    });

    web_sockets_.start();
    worker_ = std::thread(&telemetry_backend::worker_loop, this);
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
            LOG_WARN("Send failed:");
            is_connected = false;
            return;
        }

        LOG_INFO("Sent command: {}", text);
        return;
    }

    LOG_WARN("Failed to send message as WebSocket was not open");
}

void telemetry_backend::worker_loop() {
    while (!should_kill_) {
        std::this_thread::sleep_for(10ms);
        if (web_sockets_.getReadyState() != ix::ReadyState::Open) { is_connected = false; }
        if (std::chrono::steady_clock::now() - last_data_time_ > 500ms) { is_receiving = false; }
    }
}

void telemetry_backend::on_message(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        buffer_.append(msg->str);
        size_t newline_pos;
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
            const std::scoped_lock<std::mutex> lock{data_mutex};

            // write data
            if (!is_logging) { continue; }

            for (const auto& [ident, value] : parsed.value()) {
                data.write_data(std::string{ident}, std::string{value});
            }
            data.write_raw_line(line);
            for (const auto& [ident, value] : parsed.value()) {
                if (ident == "W") {
                    data.save_current_line(line);
                    break;
                }
            }
        }
    }
}

// This function serves to handle the incoming message recovered from OnMEssage.
// The goal is to primarly handle the data packing itself and doesn't have any command
// or response handling. See "HandleResponse" for response logic
std::optional<std::vector<std::pair<std::string_view, std::string_view>>>
telemetry_backend::validate_packet(std::string_view str) const {
    std::vector<std::pair<std::string_view, std::string_view>> parsed;
    parsed.reserve(data.data_values.size());

    size_t pos = 0;
    // runs through the whole sent packet. this ensures that the packet must be valid but doesn't
    // need every m_packetfield.
    while (pos < str.size()) {
        bool         is_key      = false;
        const size_t ident_start = pos;

        // run until space to find key
        while (pos < str.size() && str[pos] != ' ') { pos++; }
        std::string_view key = str.substr(ident_start, pos - ident_start);

        // run until start of value
        while (pos < str.size() && str[pos] == ' ') { pos++; }
        const size_t value_start = pos;

        // run until space again to find value
        while (pos < str.size() && str[pos] != ' ') { pos++; }
        if (value_start == pos) { return std::nullopt; }
        std::string_view value = str.substr(value_start, pos - value_start);

        // skip extra spaces
        while (pos < str.size() && str[pos] == ' ') { pos += 1; }

        // check key value pairs
        for (const auto& field : data.data_values) {
            if (field.key == key) { is_key = true; }
        }
        if (!is_key) { return std::nullopt; }
        if (key == "T") {
            uint64_t t   = 0;
            auto     res = std::from_chars(value.data(), value.data() + value.size(), t);
            if (res.ec != std::errc{} || t == 0) { return std::nullopt; }
        }
        parsed.emplace_back(key, value);
    }
    bool has_t = false;
    for (const auto& pair : parsed) {
        if (pair.first == "T") {
            has_t = true;
            break;
        }
    }
    if (!has_t) { return std::nullopt; }

    return parsed;
}

telemetry_data::packed_data telemetry_backend::pack_data() {
    const std::scoped_lock<std::mutex> lock{data_mutex};
    return {.time_micros_raw         = data.get_time_no_normal(),
            .time_minutes_normalized = data.get_time(),
            .series                  = data.series,
            .raw_lines               = data.get_raw_lines()};
}

void telemetry_backend::set_ip(const ipv4_t& ipv4) {
    if (!ipv4.is_valid()) {
        LOG_ERROR("Requested Ip was invalid: {}", ipv4.to_string());
        return;
    }

    ip_addr_ = ipv4;
    kill();
    should_kill_ = false;
    start();
    try_connection = false;
}
// This function is the bridge between any RES sent from our car to our app
// After checking for RES in OnMessage, this function is called to parse and act on the response
// The response is parsed by checking the string with the response type ENUM and calling the
// corresponding handler For reference, general responses look like this: RES CMD_TYPE CMD_PAYLOAD
void telemetry_backend::handle_response(std::string_view line) {
    constexpr size_t res_length = 4;
    if (line.size() <= res_length) {
        LOG_ERROR("Invalid response length: {}", line.size());
        return;
    }
    auto rest  = line.substr(res_length);
    auto space = rest.find(' ');
    if (space == std::string::npos) {
        LOG_ERROR("No space after response type: {}", rest);
        return;
    }

    auto            command = rest.substr(0, space);
    response_type_t type    = res_string_to_enum(command);
    if (type == response_type_t::UNKNOWN) {
        LOG_ERROR("Unknown response: {}", command);
        return;
    }
    auto it = response_handlers.find(type);
    if (it != response_handlers.end()) {
        it->second(rest.substr(space + 1));
    } else {
        LOG_ERROR("No handler for response: {}", command);
    }
}

// ALL responses should be created with a lambda that takes a std::string_view and returns void
// The lambda should parse the response and log any errors or success messages
// ENUMs are created in backend.hpp so when you add a new response, add it there first
// This also requires additions to ResStringtoEnum as these are ENUM mapped lambdas,
// not just a simple string mapped lambda
void telemetry_backend::register_handlers() {
    response_handlers[response_type_t::SYNC] = [](std::string_view line) {
        uint64_t micros = 0;
        std::from_chars(line.data(), line.data() + line.size(), micros);
        local_time t{micros};
        LOG_INFO("Time successfully synced at: {}", t.to_string(false));
    };

    response_handlers[response_type_t::SDSTART] = [this](std::string_view line) {
        if (line == "deadbeef") {
            LOG_ERROR("SD Card Failed Initialization");
        } else {
            LOG_INFO("SD Card Initialized At {}", line);
            is_open = true;
        }
    };

    response_handlers[response_type_t::SDWRITE] = [this](std::string_view line) {
        if (line == "1") {
            LOG_INFO("SD Card Has Begun Writing");
            is_writing = true;
        } else if (line == "0") {
            LOG_INFO("SD Card Has Stopped Writing");
            is_writing = false;
        }
    };

    response_handlers[response_type_t::SDCLOSE] = [this](std::string_view line) {
        if (line == "1") {
            is_open = false;
            LOG_INFO("SD Card Has Closed Succesfully");
        } else if (line == "0") {
            LOG_INFO("SD Card Failed Close");
        }
    };
}
// As mentioned before, you will need to update this string to enum function
// EVERY TIME you add a new command. This works in series with RegisterHandlers
// and HandleResponse
response_type_t telemetry_backend::res_string_to_enum(std::string_view command) const {
    static const std::unordered_map<std::string_view, response_type_t> lookup{
        {"SYNC", response_type_t::SYNC},
        {"SD_START", response_type_t::SDSTART},
        {"SD_WRITE", response_type_t::SDWRITE},
        {"SD_CLOSE", response_type_t::SDCLOSE},
    };
    auto it = lookup.find(command);
    return it != lookup.end() ? it->second : response_type_t::UNKNOWN;
}

/*
auto TelemetryBackend::HandleCommand(
    std::optional<std::vector<std::pair<std::string_view, std::string_view>>> parsed) -> void {
    if (!parsed || parsed->size() < 2) {
        LOG_ERROR("Command Not Found");
        return;
    }
    auto response = parsed.value()[1];
    if (response.first == "SYNC") {
        uint64_t micros = 0;
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
    const size_t ident_start = pos;
    while (pos < str.size() && str[pos] != ' ') {
        pos += 1;
    }
    if (str.substr(ident_start, pos - ident_start) != field) { return std::nullopt; }

    // There can be an arbitrary amount of spaces between idents/values
    if (pos >= str.size() || str[pos] != ' ') { return std::nullopt; }
    while (pos < str.size() && str[pos] == ' ') {
        pos += 1;
    }

    const size_t value_start = pos;
    while (pos < str.size() && str[pos] != ' ') {
        pos += 1;
    }
    if (value_start == pos) { return std::nullopt; }
    const auto value = str.substr(value_start, pos - value_start);

    // Uncalibrated packets or unparsable times are invalid
    if (field == "T") {
        uint64_t t   = 0;
        auto     res = std::from_chars(value.data(), value.data() + value.size(), t);
        if (res.ec != std::errc{} || t == 0) { return std::nullopt; }
    }

    parsed.emplace_back(field, value);
    while (pos < str.size() && str[pos] == ' ') {
        pos += 1;
    }
}
*/

} // namespace mbr
