#include <cassert>
#include <charconv>
#include <format>
#include <string>

#include <ixwebsocket/IXNetSystem.h>

#include "app/context.hpp"

#include "esp32/backend.hpp"

#include "core/log.hpp"

using namespace std::chrono_literals;

TelemetryBackend::TelemetryBackend(std::vector<std::string> packet_fields)
    : SerialMan(115200, 500) {
    m_PacketFields = std::move(packet_fields);
    m_Buffer.reserve(4096);
    m_IpAddr = DEFAULT_IP;
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
            const auto parsed = ValidatePacket(line);
            if (!parsed) { continue; }

            // Now we can safely unpack the packet
            const std::scoped_lock<std::mutex> lock{DataMutex};

            // check for commands/responses first
            auto pair = parsed.value()[0];
            if (pair.first == "RES") {
                HandleCommand(parsed);
                continue;
            }

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

std::optional<std::vector<std::pair<std::string_view, std::string_view>>>
TelemetryBackend::ValidatePacket(std::string_view str) const {
    std::vector<std::pair<std::string_view, std::string_view>> parsed;
    parsed.reserve(m_PacketFields.size());

    size_t pos = 0;
    // runs through the whole sent packet. this ensures that the packet must be valid but doesn't
    // need every m_packetfield.
    while (pos < str.size()) {
        bool         isKey       = 0;
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
        for (const auto& field : m_PacketFields) {
            if (field == key) { isKey = 1; }
        }
        if (!isKey) { return std::nullopt; }
        if (key == "T") {
            uint64_t t   = 0;
            auto     res = std::from_chars(value.data(), value.data() + value.size(), t);
            if (res.ec != std::errc{} || t == 0) { return std::nullopt; }
        }
        parsed.emplace_back(key, value);
    }
    bool hasT   = 0;
    bool hasRES = 0;
    for (const auto& pair : parsed) {
        if (pair.first == "T") {
            hasT = 1;
            break;
        }
        if (pair.first == "RES") {
            hasRES = 1;
            break;
        }
    }
    if (hasT == hasRES) { return std::nullopt; }

    return parsed;
}

TelemetryData::PackedData TelemetryBackend::PackData() {
    const std::scoped_lock<std::mutex> lock{DataMutex};
    return {.TimeMicrosRaw         = Data.GetTimeNoNormal(),
            .TimeMinutesNormalized = Data.GetTime(),
            .RPM                   = Data.GetRPMData(),
            .Shock                 = Data.GetShockData(),
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
/*
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
