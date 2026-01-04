#ifndef BOARD_WIFI_HPP
#define BOARD_WIFI_HPP

#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
// #include <DNSServer.h>

class BoardWifi {
  public:
    BoardWifi(const char* ssid, const char* password);
    void          cleanupClients() { _ws.cleanupClients(); }
    void          Start();
    void          ReceiveCMD();
    void          SendData(String msg);
    String        lastCommand = "";
    volatile bool newCommand  = false;
    uint64_t      getRealTime();
    void          updateDNS();

  private:
    const char*   _ssid;
    const char*   _password;
    uint64_t      baseTimeMicros  = 0;
    uint32_t      localSyncMicros = 0;
    volatile bool isTimeSynced    = 0;
    volatile bool timeCMD         = 0;
    // DNSServer _dnsServer;

    AsyncWebServer _server;
    AsyncWebSocket _ws;

    static void onWsEvent(AsyncWebSocket*       server,
                          AsyncWebSocketClient* client,
                          AwsEventType          type,
                          void*                 arg,
                          uint8_t*              data,
                          size_t                len);
};

#endif
