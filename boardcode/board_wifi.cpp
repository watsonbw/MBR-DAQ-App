#include "board_wifi.hpp"

BoardWifi* BoardWifi::s_Instance = nullptr;

BoardWifi::BoardWifi(const char* ssid, const char* password)
    : m_SSID(ssid), m_Password(password), m_AsyncServer(80), m_WebSock("/ws") {
    s_Instance = this;
}

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

    m_WebSock.onEvent(BoardWifi::OnWsEvent);
    
    m_AsyncServer.addHandler(&m_WebSock);

    m_AsyncServer.on("/connecttest.txt", [](AsyncWebServerRequest* request) {
        request->send(200, "text/plain", "Microsoft NCSI");
    });

    m_AsyncServer.on("/generate_204", [](AsyncWebServerRequest* request) { request->send(204); });
    m_AsyncServer.begin();
    Serial.println("HTTP Server started");
}

void BoardWifi::SendData(String msg) {
    if (m_WebSock.count() > 0 ) { m_WebSock.textAll(msg); }
}

void BoardWifi::OnWsEvent(AsyncWebSocket*       server,
                          AsyncWebSocketClient* client,
                          AwsEventType          type,
                          void*                 arg,
                          uint8_t*              data,
                          size_t                len) {
    if (type == WS_EVT_DATA && s_Instance != nullptr) {
        String command = "";
        for (size_t i = 0; i < len; i++) {
            command += (char)data[i];
        }

        if (command.startsWith("SYNC")) {
            String timeStr            = command.substring(4);
            s_Instance->m_LocalSyncMicros = micros();
            s_Instance->m_BaseTimeMicros  = strtoull(timeStr.c_str(), NULL, 10);
            s_Instance->m_IsTimeSynced    = 1;
        } else {
            s_Instance->m_LastCommand = command;
            s_Instance->m_NewCommand  = true;
        }

        Serial.print("Received Command: ");
        Serial.println(command);
    } else if (type == WS_EVT_CONNECT) {
       server->cleanupClients();
        Serial.println("Client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    }
}

uint64_t BoardWifi::GetRealTime() {
    if (!s_Instance->m_IsTimeSynced) {return 0;}
    uint32_t elapsedMicros = micros() - m_LocalSyncMicros;
    return m_BaseTimeMicros + (uint64_t)(elapsedMicros);
}
