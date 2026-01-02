#include "board_wifi.hpp"


BoardWifi::BoardWifi(const char* ssid, const char* password) 
    : _ssid(ssid), _password(password), _server(81), _ws("/ws") {}

void BoardWifi::Start(){
    WiFi.softAP(_ssid, _password);

    Serial.print("WiFi IP address: ");
    Serial.println(WiFi.softAPIP());

    if (MDNS.begin("telemetry")) {
        Serial.println("mDNS responder started");
    }

    _ws.onEvent(onWsEvent);
    _server.addHandler(&_ws);

    _server.begin();
    Serial.println("HTTP Server started");

}

void BoardWifi::SendData(String msg) {
    _ws.textAll(msg);
}

void BoardWifi::onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
        Serial.printf("Received: %.*s\n", (int)len, (char*)data);
    } 
    else if (type == WS_EVT_CONNECT) {
        Serial.println("Client connected");
    } 
    else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    }
}