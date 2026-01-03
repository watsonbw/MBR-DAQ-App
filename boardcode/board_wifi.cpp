#include "board_wifi.hpp"

static BoardWifi* instance = nullptr;

BoardWifi::BoardWifi(const char* ssid, const char* password)
    : _ssid(ssid), _password(password), _server(80), _ws("/ws") {
    instance = this;
}

void BoardWifi::Start() {

    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_AP);
    delay(100);

    IPAddress local_IP(192,168,4,1);
    IPAddress gateway(192,168,4,1);
    IPAddress subnet(255,255,255,0);
    WiFi.softAPConfig(local_IP, gateway, subnet);

    if (WiFi.softAP(_ssid, _password, 1, 0, 4)) {
        Serial.println("SoftAP Started Successfully");
    } else {
        Serial.println("SoftAP Failed to Start");
    }
    Serial.print("WiFi IP address: ");
    Serial.println(WiFi.softAPIP());



    if (MDNS.begin("telemetry")) { Serial.println("mDNS responder started"); }

    _ws.onEvent(onWsEvent);
    _server.addHandler(&_ws);

    _server.on("/connecttest.txt", [](AsyncWebServerRequest *request){
        request->send(200, "text/plain", "Microsoft NCSI");
    });
    
    _server.on("/generate_204", [](AsyncWebServerRequest *request){
        request->send(204);
    });

    _server.begin();
    Serial.println("HTTP Server started");
    
}

void BoardWifi::SendData(String msg) {
    if (_ws.count() > 0 && _ws.availableForWriteAll()) {
        _ws.textAll(msg);
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
        for (size_t i = 0; i < len; i++) {
            command += (char)data[i];
        }

        if (command.startsWith("SYNC")) {
            String timeStr            = command.substring(4);
            instance->localSyncMicros = micros();
            instance->baseTimeMicros  = strtoull(timeStr.c_str(), NULL, 10);
            instance->isTimeSynced    = 1;
        } else {
            instance->lastCommand = command;
            instance->newCommand  = true;
        }

        Serial.print("Received Command: ");
        Serial.println(command);
    } else if (type == WS_EVT_CONNECT) {
        for (auto& c : server->getClients()) {
            if (server->count() > 3) {
                server->cleanupClients(); 
            }
        }
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
/*
void BoardWifi::updateDNS() {
    _dnsServer.processNextRequest();
}
*/
