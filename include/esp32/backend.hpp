#pragma once

#include <ixwebsocket/IXWebSocket.h>
#include <memory>
#include <thread>

struct AppContext;

class TelemetryBackend {
  public:
    explicit TelemetryBackend(std::shared_ptr<AppContext> ctx);
    ~TelemetryBackend();

    void Start();

  private:
    void WorkerLoop();

  private:
    std::shared_ptr<AppContext> m_Context;
    std::thread                 m_Worker;
};
