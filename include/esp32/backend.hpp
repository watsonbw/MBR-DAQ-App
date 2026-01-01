#pragma once

#include <memory>
#include <thread>

#include <ixwebsocket/IXWebSocket.h>

struct AppContext;

class TelemetryBackend {
  public:
    explicit TelemetryBackend(std::shared_ptr<AppContext> ctx);
    ~TelemetryBackend();

    void Start();
    void Kill();

  private:
    void WorkerLoop();
    void OnMessage(const ix::WebSocketMessagePtr& msg);

  private:
    std::shared_ptr<AppContext> m_Context;
    std::thread                 m_Worker;
    ix::WebSocket               m_WebSocket;
};
