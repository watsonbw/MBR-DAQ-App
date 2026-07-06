#pragma once

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
// #include <DNSServer.h>

class BoardWifi {
  public:
    explicit BoardWifi(const char* ssid, const char* password);
    ~BoardWifi() = default;

    void          CleanupClients() { m_WebSock.cleanupClients(); }
    void          Start();
    void          SendData(const char* msg);
    char          CommandValue[128]{""};
    volatile bool NewCommand{false};

  private:
    const char* m_SSID;
    const char* m_Password;

    AsyncWebServer m_AsyncServer;
    AsyncWebSocket m_WebSock;
};


