#include <cassert>
#include <charconv>
#include <format>
#include <math.h>
#include <string>

#include <ixwebsocket/IXNetSystem.h>
#include <type_traits>

#include "app/context.hpp"

#include "esp32/backend.hpp"

#include "core/log.hpp"

using namespace std::chrono_literals;

TelemetryBackend::TelemetryBackend()
    : SerialMan(115200, 500) {
    m_Buffer.reserve(4096);
    m_IpAddr = DEFAULT_IP;
    RegisterHandlers();
}

TelemetryBackend::~TelemetryBackend() { Kill(); }

void TelemetryBackend::Start() {
    LOG_INFO("Started Connection Attempt");
    Kill();
    m_ShouldKill = false;

    assert(m_IpAddr.Valid());
    const auto real_addr = std::format("ws://{}/ws", m_IpAddr.String());
    LOG_INFO("Attempting to connect with address: {}", real_addr);
    m_WebSocket.setUrl(real_addr);

    m_WebSocket.setMaxWaitBetweenReconnectionRetries(5000);
    m_WebSocket.setMinWaitBetweenReconnectionRetries(1000);
    m_WebSocket.enableAutomaticReconnection();
    // m_WebSocket.setPingInterval(30);
    m_WebSocket.setHandshakeTimeout(10);

    m_WebSocket.setOnMessageCallback([this](const ix::WebSocketMessagePtr& msg) {
        if (msg->type == ix::WebSocketMessageType::Open) {
            IsConnected = true;
            IsReceiving = false;
            SendCMD("STATUS");
            LOG_INFO("Connected to ESP32");
        }

        if (msg->type == ix::WebSocketMessageType::Close ||
            msg->type == ix::WebSocketMessageType::Error) {
            IsConnected = false;
            IsReceiving = false;
        }
        if (msg->type == ix::WebSocketMessageType::Close) { LOG_WARN("WebSocket closed"); }

        if (msg->type == ix::WebSocketMessageType::Error) {
            LOG_ERROR("WebSocket error: {}", msg->errorInfo.reason.c_str());
            LOG_ERROR("HTTP Status: {}", msg->errorInfo.http_status);
        }

        if (msg->type == ix::WebSocketMessageType::Message) {
            IsReceiving = true;
            this->OnMessage(msg);
            m_LastDataTime = std::chrono::steady_clock::now();
        }
    });

    m_WebSocket.start();
    m_Worker = std::thread(&TelemetryBackend::WorkerLoop, this);
}

void TelemetryBackend::Kill() {
    m_WebSocket.stop();
    if (m_Worker.joinable()) {
        m_ShouldKill = true;
        m_Worker.join();
    }
}

void TelemetryBackend::SendCMD(const std::string& text) {
    if (m_WebSocket.getReadyState() == ix::ReadyState::Open) {
        const ix::WebSocketSendInfo info = m_WebSocket.send(text);
        if (!info.success) {
            LOG_WARN("Send failed:");
            IsConnected = false;
            return;
        }

        LOG_INFO("Sent command: {}", text);
        return;
    }

    LOG_WARN("Failed to send message as WebSocket was not open");
}

void TelemetryBackend::WorkerLoop() {
    while (!m_ShouldKill) {
        std::this_thread::sleep_for(10ms);
        if (m_WebSocket.getReadyState() != ix::ReadyState::Open) { IsConnected = false; }
        if (std::chrono::steady_clock::now() - m_LastDataTime > 500ms) { IsReceiving = false; }
    }
}

void TelemetryBackend::OnMessage(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        m_Buffer.append(msg->str);
        size_t newline_pos;
        while ((newline_pos = m_Buffer.find('\n')) != std::string::npos) {
            const auto line = m_Buffer.substr(0, newline_pos);
            m_Buffer.erase(0, newline_pos + 1);

            // check for commands/responses first
            if (line.starts_with("RES")) {
                HandleResponse(line);
                continue;
            }

            const auto parsed = ValidatePacket(line);
            if (!parsed) { continue; }

            // Now we can safely unpack the packet
            const std::scoped_lock<std::mutex> lock{DataMutex};


            // write data
            if (!IsLogging) { continue; }

            for (const auto& [ident, value] : parsed.value()) {
                Data.WriteData(std::string{ident}, std::string{value});
            }
            Data.WriteRawLine(line);
            for (const auto& [ident, value] : parsed.value()) {
                if (ident == "W") {
                    Data.SaveCurrentLine(line);
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
TelemetryBackend::ValidatePacket(std::string_view str) const {
    std::vector<std::pair<std::string_view, std::string_view>> parsed;
    parsed.reserve(Data.DataValues.size());

    size_t pos = 0;
    // runs through the whole sent packet. this ensures that the packet must be valid but doesn't
    // need every m_packetfield.
    while (pos < str.size()) {
        bool         is_key       = false;
        const size_t ident_start = pos;

        // run until space to find key
        while (pos < str.size() && str[pos] != ' ') {
            pos++;
        }
        std::string_view key = str.substr(ident_start, pos - ident_start);

        // run until start of value
        while (pos < str.size() && str[pos] == ' ') {
            pos++;
        }
        const size_t value_start = pos;

        // run until space again to find value
        while (pos < str.size() && str[pos] != ' ') {
            pos++;
        }
        if (value_start == pos) { return std::nullopt; }
        std::string_view value = str.substr(value_start, pos - value_start);

        // skip extra spaces
        while (pos < str.size() && str[pos] == ' ') {
            pos += 1;
        }

        // check key value pairs
        for (const auto& field : Data.DataValues) {
            if (field.Key == key) { is_key = true; }
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

TelemetryData::PackedData TelemetryBackend::PackData() {
    const std::scoped_lock<std::mutex> lock{DataMutex};
    return {.TimeMicrosRaw         = Data.GetTimeNoNormal(),
            .TimeMinutesNormalized = Data.GetTime(),
            .Series                = Data.Series,
            .RawLines              = Data.GetRawLines()};
}

void TelemetryBackend::SetIp(const IpV4& ipv4) {
    if (!ipv4.Valid()) {
        LOG_ERROR("Requested Ip was invalid: {}", ipv4.String());
        return;
    }

    m_IpAddr = ipv4;
    Kill();
    m_ShouldKill = false;
    Start();
    TryConnection = false;
}
// This function is the bridge between any RES sent from our car to our app
// After checking for RES in OnMessage, this function is called to parse and act on the response
// The response is parsed by checking the string with the response type ENUM and calling the corresponding handler
// For reference, general responses look like this: RES CMD_TYPE CMD_PAYLOAD
void TelemetryBackend::HandleResponse(std::string_view line) {
    constexpr size_t res_length = 4;
    if (line.size() <= res_length) {
        LOG_ERROR("Invalid response length: {}", line.size());
        return;
    }
    auto rest = line.substr(res_length);
    auto space = rest.find(' ');
    if (space == std::string::npos) {
        LOG_ERROR("No space after response type: {}", rest);
        return;
    }

    auto command = rest.substr(0, space);
    ResponseType type = ResStringToEnum(command);
    if (type == ResponseType::UNKNOWN) {
        LOG_ERROR("Unknown response: {}", command);
        return;
    }
    auto it = m_ResponseHandlers.find(type);
    if (it != m_ResponseHandlers.end()) {
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
void TelemetryBackend::RegisterHandlers() {
    m_ResponseHandlers[ResponseType::SYNC] = [this](std::string_view line) {
        uint64_t micros = 0;
        std::from_chars(
            line.data(), line.data() + line.size(), micros);
        LocalTime t{micros};
        LOG_INFO("Time successfully synced at: {}", t.String(false));
    };

    m_ResponseHandlers[ResponseType::SDSTART] = [this](std::string_view line) {
        if (line == "deadbeef") {
            LOG_ERROR("SD Card Failed Initialization");
        } else {
            LOG_INFO("SD Card Initialized At {}", line);
            IsOpen = true;
        }
    };

    m_ResponseHandlers[ResponseType::SDWRITE] = [this](std::string_view line) {
        if (line == "1") {
            LOG_INFO("SD Card Has Begun Writing");
            IsWriting = true;
        } else if (line == "0") {
            LOG_INFO("SD Card Has Stopped Writing");
            IsWriting = false;
        }
    };

    m_ResponseHandlers[ResponseType::SDCLOSE] = [this](std::string_view line) {
        if (line == "1") {
            IsOpen = false;
            LOG_INFO("SD Card Has Closed Succesfully");
        } else if (line == "0") {
            LOG_INFO("SD Card Failed Close");
        }
    };
}
// As mentioned before, you will need to update this string to enum function
// EVERY TIME you add a new command. This works in series with RegisterHandlers
// and HandleResponse
ResponseType TelemetryBackend::ResStringToEnum(std::string_view command) const {
    static const std::unordered_map<std::string_view, ResponseType> lookup{
        {"SYNC",     ResponseType::SYNC},
        {"SD_START", ResponseType::SDSTART},
        {"SD_WRITE", ResponseType::SDWRITE},
        {"SD_CLOSE", ResponseType::SDCLOSE},
    };
    auto it = lookup.find(command);
    return it != lookup.end() ? it->second : ResponseType::UNKNOWN;
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
