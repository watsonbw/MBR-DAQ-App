#include <sstream>
#include <string>

#include <ixwebsocket/IXNetSystem.h>

#include "app/context.hpp"

#include "esp32/backend.hpp"

TelemetryBackend::TelemetryBackend(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {}

TelemetryBackend::~TelemetryBackend() { Kill(); }

void TelemetryBackend::Start() {
    Kill();
    m_ShouldKill = false;

    ix::initNetSystem();
    m_WebSocket.setUrl("ws://telemetry.local:81");
    m_WebSocket.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr& msg) { this->OnMessage(msg); });
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
    m_Context->IsConnected = true;
    while (!m_ShouldKill) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (m_WebSocket.getReadyState() != ix::ReadyState::Open) { m_Context->IsConnected = false; }
    }
}

void TelemetryBackend::OnMessage(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        std::lock_guard<std::mutex> lock(m_Context->DataMutex);
        std::stringstream           ss(msg->str);

        std::string identifier;
        std::string value;
        m_Context->Data.WriteRawLine(msg->str);
        while (ss >> identifier >> value) {
            m_Context->Data.WriteData(identifier, value);
        }
    }
}

void TelemetryBackend::SendCMD(const std::string& text) {
    if (m_WebSocket.getReadyState() == ix::ReadyState::Open) { m_WebSocket.send(text); }
}
