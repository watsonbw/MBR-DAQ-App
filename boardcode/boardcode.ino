#include "board_wifi.hpp"

static const char*            ssid     = "esp32Wifi";
static const char*            password = "MBRdatacollect";
static BoardWifi              wifi{ssid, password};
static volatile unsigned long last_cleanup = 0;
static unsigned long          last_send    = 0;

void setup() {
    Serial.begin(115200);
    wifi.Start();
    wifi.CleanupClients();

    
}

void loop() {
    // wifi.updateDNS();
    if (millis() - last_cleanup > 10000) {
        wifi.CleanupClients();
        last_cleanup = millis();
    }

    if (micros() - last_send > 50000) {
        const uint64_t real_time = wifi.GetRealTime();
        wifi.SendData("T " + String(real_time) + " W 2300 E 300 fr 0 fl 0 br 0 bl 0\n");
        last_send = micros();
    }
}
