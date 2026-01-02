#include <sstream>
#include <string>

#include <ixwebsocket/IXNetSystem.h>

#include "app/context.hpp"

#include "esp32/backend.hpp"

#include "core/log.hpp"

TelemetryBackend::~TelemetryBackend() { Kill(); }

void TelemetryBackend::Start() {
    LOG_INFO("Started Connection Attempt");

    auto state = m_WebSocket.getReadyState();
    if (state == ix::ReadyState::Open || state == ix::ReadyState::Connecting) {
        return;
    }
    LOG_INFO("Start Function Success");
    Kill();
    
    m_ShouldKill = false;
    
    
    m_WebSocket.setUrl("ws://192.168.4.1:81/ws");
 
    m_WebSocket.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) { 
            
            if (msg->type == ix::WebSocketMessageType::Open)
            IsConnected = true;

            if (msg->type == ix::WebSocketMessageType::Close ||
            msg->type == ix::WebSocketMessageType::Error)
            IsConnected = false;

            if (msg->type == ix::WebSocketMessageType::Message)
            this->OnMessage(msg);
        
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
        Data.WriteRawLine(msg->str);
        while (ss >> identifier >> value) {
            Data.WriteData(identifier, value);
        }
    }
}

void TelemetryBackend::SendCMD(const std::string& text) {
    if (m_WebSocket.getReadyState() == ix::ReadyState::Open) { m_WebSocket.send(text); }
}
