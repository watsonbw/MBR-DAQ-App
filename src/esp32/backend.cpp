#include <sstream>
#include <string>

#include <ixwebsocket/IXNetSystem.h>

#include "esp32/backend.hpp"
#include "app/context.hpp"

TelemetryBackend::TelemetryBackend(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {}

TelemetryBackend::~TelemetryBackend() {
    if (m_Worker.joinable()) { m_Worker.join(); }
}

void TelemetryBackend::Start() { 
    ix::initNetSystem();
    m_WebSocket.setUrl("ws://telemetry.local:81"); 
    m_WebSocket.start();
    
    m_Worker = std::thread(&TelemetryBackend::WorkerLoop, this); 
}

void TelemetryBackend::Kill() {}

void TelemetryBackend::WorkerLoop() {
    while (!m_Context->ShouldExit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void TelemetryBackend::OnMessage(const ix::WebSocketMessagePtr& msg) {
    if (msg->type == ix::WebSocketMessageType::Message) {
        std::stringstream ss(msg->str);

        std::string identifier;
        std::string value;

        while (ss >> identifier >> value) { m_Context->Data.WriteData(identifier, value);}
    }
}
