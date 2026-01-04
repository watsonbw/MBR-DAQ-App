#include <sstream>
#include <string>

#include <ixwebsocket/IXNetSystem.h>

#include "app/context.hpp"

#include "esp32/backend.hpp"

#include "core/log.hpp"

TelemetryBackend::~TelemetryBackend() { Kill(); }

void TelemetryBackend::Start() {
    LOG_INFO("Started Connection Attempt");

    // auto state = m_WebSocket.getReadyState();
    // if (state == ix::ReadyState::Open || state == ix::ReadyState::Connecting) { return; }
    LOG_INFO("Start Function Success");
    Kill();

    m_ShouldKill = false;

    m_WebSocket.setUrl("ws://192.168.4.1:80/ws");

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
            msg->type == ix::WebSocketMessageType::Error){
            IsConnected = false;
            IsReceiving = false;
            }
        if (msg->type == ix::WebSocketMessageType::Close) { LOG_WARN("WebSocket closed"); }

        if (msg->type == ix::WebSocketMessageType::Error) {
            LOG_ERROR("WebSocket error: {}", msg->errorInfo.reason.c_str());
            LOG_ERROR("HTTP Status: {}", msg->errorInfo.http_status);
        }

        if (msg->type == ix::WebSocketMessageType::Message){
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

void TelemetryBackend::WorkerLoop() {
    while (!m_ShouldKill) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (m_WebSocket.getReadyState() != ix::ReadyState::Open) { IsConnected = false; }
    }
}

void TelemetryBackend::OnMessage(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        std::lock_guard<std::mutex> lock(DataMutex);
        std::stringstream           ss(msg->str);
       
        


        std::string identifier;
        std::string value;
        if (IsLogging) {
            while (ss >> identifier >> value) {
                // LOG_INFO("Parsed: Key='{}' Val='{}'", identifier, value);
                if (identifier == "T" && std::stoull(value) == 0) { return; }
                Data.WriteData(identifier, value);
            }
            Data.WriteRawLine(msg->str);
        }
    }
}

void TelemetryBackend::SendCMD(const std::string& text) {
    if (m_WebSocket.getReadyState() == ix::ReadyState::Open) {
        ix::WebSocketSendInfo info = m_WebSocket.send(text);
        if (!info.success) {
            LOG_WARN("Send failed:");
            IsConnected = false;
        }
    }
}
