#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include <ixwebsocket/IXWebSocket.h>

#include "core/ip.hpp"

#include "esp32/data.hpp"

struct AppContext;

class TelemetryBackend {
  public:
    TelemetryBackend() = delete;
    explicit TelemetryBackend(std::vector<std::string> packet_fields);
    ~TelemetryBackend();

    void Start();
    void Kill();
    void SendCMD(const std::string& text);

    // Returns a safely accessible grouping of relevant data.
    //
    // This automatically manages the Data's mutex!
    TelemetryData::PackedData PackData();
    void                      SetIp(const IpV4& ipv4);

  public:
    std::mutex        DataMutex;
    TelemetryData     Data;
    std::atomic<bool> TryConnection{false};
    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};
    std::atomic<bool> IsReceiving{false};

  private:
    void WorkerLoop();
    void OnMessage(const ix::WebSocketMessagePtr& msg);

    std::optional<std::vector<std::pair<std::string_view, std::string_view>>>
    ValidatePacket(std::string_view str) const;

  private:
    std::thread              m_Worker;
    ix::WebSocket            m_WebSocket;
    std::string              m_Buffer;
    std::vector<std::string> m_PacketFields;
    IpV4                     m_IpAddr;

    std::atomic<bool> m_ShouldKill{false};
};
