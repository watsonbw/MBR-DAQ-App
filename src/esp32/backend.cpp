#include <cassert>
#include <charconv>
#include <chrono>
#include <format>
#include <string>

#include <ixwebsocket/IXNetSystem.h>

#include "app/context.hpp"

#include "esp32/backend.hpp"

#include "core/log.hpp"

using namespace std::chrono_literals;

TelemetryBackend::TelemetryBackend(std::vector<std::string> packet_fields) {
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
        ix::WebSocketSendInfo info = m_WebSocket.send(text);
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
    }
}

void TelemetryBackend::OnMessage(const ix::WebSocketMessagePtr& msg) {
    if (IsLogging && msg->type == ix::WebSocketMessageType::Message) {
        m_Buffer.append(msg->str);
        size_t newline_pos;
        while ((newline_pos = m_Buffer.find('\n')) != std::string::npos) {
            const auto line = m_Buffer.substr(0, newline_pos);
            m_Buffer.erase(0, newline_pos + 1);
            const auto parsed = ValidatePacket(line);
            if (!parsed.has_value()) { continue; }

            // Now we can safely unpack the packet
            std::lock_guard<std::mutex> lock{DataMutex};
            for (const auto& [ident, value] : parsed.value()) {
                Data.WriteData(std::string{ident}, std::string{value});
            }
            Data.WriteRawLine(line);
        }
    }
}

std::optional<std::vector<std::pair<std::string_view, std::string_view>>>
TelemetryBackend::ValidatePacket(std::string_view str) const {
    std::vector<std::pair<std::string_view, std::string_view>> parsed;
    parsed.reserve(m_PacketFields.size());

    size_t pos = 0;
    for (const auto& field : m_PacketFields) {
        size_t ident_start = pos;
        while (pos < str.size() && str[pos] != ' ') {
            pos += 1;
        }
        if (str.substr(ident_start, pos - ident_start) != field) { return std::nullopt; }

        // There can be an arbitrary amount of spaces between idents/values
        if (pos >= str.size() || str[pos] != ' ') { return std::nullopt; }
        while (pos < str.size() && str[pos] == ' ') {
            pos += 1;
        }

        size_t value_start = pos;
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

    // Consume trailing whitespace just to be safe
    while (pos < str.size() && str[pos] == ' ') {
        pos += 1;
    }
    if (pos != str.size()) { return std::nullopt; }
    return parsed;
}

TelemetryData::PackedData TelemetryBackend::PackData() {
    std::lock_guard<std::mutex> lock{DataMutex};
    return {Data.GetTimeNoNormal(),
            Data.GetTime(),
            Data.GetRPMData(),
            Data.GetShockData(),
            Data.GetRawLines()};
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
