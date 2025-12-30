#include "esp32/backend.hpp"

#include "app/context.hpp"

TelemetryBackend::TelemetryBackend(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {}

TelemetryBackend::~TelemetryBackend() {
    if (m_Worker.joinable()) { m_Worker.join(); }
}

void TelemetryBackend::Start() { m_Worker = std::thread(&TelemetryBackend::WorkerLoop, this); }

void TelemetryBackend::WorkerLoop() {
    while (!m_Context->ShouldExit) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
