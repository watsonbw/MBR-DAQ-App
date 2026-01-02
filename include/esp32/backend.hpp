#pragma once

#include <atomic>
#include <memory>
#include <thread>

#include "data.hpp"
#include <ixwebsocket/IXWebSocket.h>

struct AppContext;

class TelemetryBackend {
  public:
    explicit TelemetryBackend() = default;
    ~TelemetryBackend();

    void Start();
    void Kill();
    void SendCMD(const std::string& text);

    std::mutex        DataMutex;
    TelemetryData     Data;
    std::atomic<bool> TryConnection{false};
    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};

  private:
    void WorkerLoop();
    void OnMessage(const ix::WebSocketMessagePtr& msg);

  private:
    std::thread   m_Worker;
    ix::WebSocket m_WebSocket;

    std::atomic<bool> m_ShouldKill = false;
};
