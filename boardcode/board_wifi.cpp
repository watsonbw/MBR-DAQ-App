#include "board_wifi.hpp"

BoardWifi::BoardWifi(const char* ssid, const char* password)
    : m_SSID(ssid), m_Password(password), m_AsyncServer(80), m_WebSock("/ws") {}

void BoardWifi::Start() {
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_AP);
    delay(100);

    IPAddress local_IP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    if (WiFi.softAP(m_SSID, m_Password, 1, 0, 4)) {
        Serial.println("SoftAP Started Successfully");
    } else {
        Serial.println("SoftAP Failed to Start");
    }
    Serial.print("WiFi IP address: ");
    Serial.println(WiFi.softAPIP());

    if (MDNS.begin("telemetry")) { Serial.println("mDNS responder started"); }

    m_WebSock.onEvent([this](AsyncWebSocket*       server,
                             AsyncWebSocketClient* client,
                             AwsEventType          type,
                             void*                 arg,
                             uint8_t*              data,
                             size_t                len) {
        if (type == WS_EVT_DATA) {
            size_t copyLen = min(len, sizeof(m_CommandValue) - 1);
            memcpy(m_CommandValue, data, copyLen);
            m_CommandValue[copyLen] = '\0';
            m_NewCommand            = true;

            Serial.print("Received Command: ");
            Serial.println(m_CommandValue);
        } else if (type == WS_EVT_CONNECT) {
            server->cleanupClients();
            Serial.println("Client connected");
        } else if (type == WS_EVT_DISCONNECT) {
            Serial.println("Client disconnected");
        }
    });

    m_AsyncServer.addHandler(&m_WebSock);

    m_AsyncServer.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    m_AsyncServer.on("/generate_204", [](AsyncWebServerRequest* request) { request->send(204); });
    m_AsyncServer.begin();
    Serial.println("HTTP Server started");
}

void BoardWifi::SendData(const char* msg) {
    if (m_WebSock.count() > 0) { m_WebSock.textAll(msg); }
}
