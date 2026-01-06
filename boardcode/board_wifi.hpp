#ifndef BOARD_WIFI_HPP
#define BOARD_WIFI_HPP

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
// #include <DNSServer.h>

class BoardWifi {
  public:
    explicit BoardWifi(const char* ssid, const char* password);
    ~BoardWifi() = default;

    void     CleanupClients() { m_WebSock.cleanupClients(); }
    void     Start();
    void     ReceiveCMD();
    void     SendData(String msg);
    uint64_t GetRealTime();
    void     UpdateCommand(String command);
    // void     UpdateDNS() { _dnsServer.processNextRequest(); }

  private:
    static void OnWsEvent(AsyncWebSocket*       server,
                          AsyncWebSocketClient* client,
                          AwsEventType          type,
                          void*                 arg,
                          uint8_t*              data,
                          size_t                len);

  private:
    static BoardWifi* s_Instance;

    const char*   m_SSID;
    const char*   m_Password;
    uint64_t      m_BaseTimeMicros{0};
    uint32_t      m_LocalSyncMicros{0};
    volatile bool m_IsTimeSynced{0};
    volatile bool m_TimeCMD{0};
    // DNSServer _dnsServer;

    String        m_LastCommand{""};
    volatile bool m_NewCommand{false};

    AsyncWebServer m_AsyncServer;
    AsyncWebSocket m_WebSock;
};

#endif
