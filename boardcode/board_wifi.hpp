#ifndef BOARD_WIFI_HPP
#define BOARD_WIFI_HPP

#include <WiFi.h>
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

class BoardWifi {
    public:

    BoardWifi(const char* ssid, const char* password);
    void Start();
    void ReceiveCMD();
    void SendData(String msg);

    private:

    const char* _ssid;
    const char* _password;

    AsyncWebServer _server;
    AsyncWebSocket _ws;

    static void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
};

#endif