#pragma once

#include <atomic>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <vector>
#include <functional>
#include <unordered_map>

#include <ixwebsocket/IXWebSocket.h>

#include "core/ip.hpp"

#include "esp32/data.hpp"
#include "esp32/serial.hpp"

struct AppContext;

enum class ResponseType {
    SYNC,
    SDSTART,
    SDWRITE,
    SDCLOSE,
    UNKNOWN,
};

class TelemetryBackend {
  public:
    explicit TelemetryBackend(std::vector<std::string> packet_fields);
    ~TelemetryBackend();

    void Start();
    void Kill();
    void SendCMD(const std::string& text);

    // Returns a safely accessible grouping of relevant data.
    //
    // This automatically manages the Data's mutex!
    [[nodiscard]] TelemetryData::PackedData PackData();
    void                                    SetIp(const IpV4& ipv4);

  public:
    std::mutex        DataMutex;
    TelemetryData     Data;
    SerialManager     SerialMan;
    std::atomic<bool> TryConnection{false};
    std::atomic<bool> IsConnected{false};
    std::atomic<bool> IsLogging{false};
    std::atomic<bool> IsReceiving{false};
    std::atomic<bool> IsWriting{false};
    std::atomic<bool> IsOpen{false};

  private:
    void WorkerLoop();
    void OnMessage(const ix::WebSocketMessagePtr& msg);

    std::optional<std::vector<std::pair<std::string_view, std::string_view>>>
    ValidatePacket(std::string_view str) const;
    void HandleResponse(std::string_view line);
    void RegisterHandlers();
    ResponseType ResStringToEnum(std::string_view str) const;

  private:
    std::thread                           m_Worker;
    ix::WebSocket                         m_WebSocket;
    std::string                           m_Buffer;
    std::vector<std::string>              m_PacketFields;
    IpV4                                  m_IpAddr;
    std::chrono::steady_clock::time_point m_LastDataTime{};
    std::atomic<bool>                     m_ShouldKill{false};

    std::unordered_map<ResponseType, std::function<void(std::string_view)>> m_ResponseHandlers;
};
