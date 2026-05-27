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
    void     SendData(const char* msg);
    char        m_CommandValue[128]{""};
    volatile bool m_NewCommand{false};

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

    AsyncWebServer m_AsyncServer;
    AsyncWebSocket m_WebSock;
};

#endif
