#include "board_wifi.hpp"

static BoardWifi* instance = nullptr;

BoardWifi::BoardWifi(const char* ssid, const char* password)
    : _ssid(ssid), _password(password), _server(81), _ws("/ws") { instance = this;}

void BoardWifi::Start() {
    WiFi.softAP(_ssid, _password);

    Serial.print("WiFi IP address: ");
    Serial.println(WiFi.softAPIP());

    if (MDNS.begin("telemetry")) { Serial.println("mDNS responder started"); }

    _ws.onEvent(onWsEvent);

    _server.begin();
    Serial.println("HTTP Server started");
    _server.addHandler(&_ws);
}

void BoardWifi::SendData(String msg) {
    if (_ws.count() > 0) {
        if (_ws.availableForWriteAll()) { _ws.textAll(msg); }
    }
}


void BoardWifi::onWsEvent(AsyncWebSocket*       server,
                          AsyncWebSocketClient* client,
                          AwsEventType          type,
                          void*                 arg,
                          uint8_t*              data,
                          size_t                len) {
    if (type == WS_EVT_DATA && instance != nullptr) {
        String command = "";
        for (size_t i = 0; i < len; i++){
            command += (char)data[i];
        }

        if(command.startsWith("SYNC")){
            String timeStr = command.substring(4);
            instance->localSyncMicros = micros();
            instance->baseTimeMicros = strtoull(timeStr.c_str(), NULL, 10);
            instance->isTimeSynced = 1;
        } else {
            instance->lastCommand = command;
            instance->newCommand = true;
        }

        Serial.print("Received Command: ");
        Serial.println(command);
    } else if (type == WS_EVT_CONNECT) {
        Serial.println("Client connected");
    } else if (type == WS_EVT_DISCONNECT) {
        Serial.println("Client disconnected");
    }
}

uint64_t BoardWifi::getRealTime() {
    if (!instance->isTimeSynced) return 0;
    uint32_t elapsedMicros = micros() - localSyncMicros;
    return baseTimeMicros + (uint64_t)(elapsedMicros);
}
