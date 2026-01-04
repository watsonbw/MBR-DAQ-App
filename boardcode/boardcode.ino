#include "board_wifi.hpp"

const char*            ssid     = "esp32Wifi";
const char*            password = "MBRdatacollect";
BoardWifi              wifi(ssid, password);
volatile unsigned long lastCleanup;
void                   setup() {
    // put your setup code here, to run once:
    Serial.begin(115200);
    wifi.Start();
    wifi.cleanupClients();
}

void loop() {
    // wifi.updateDNS();
    if (millis() - lastCleanup > 10000) {
        wifi.cleanupClients();
        lastCleanup = millis();
    }

    static unsigned long lastSend = 0;
    if (micros() - lastSend > 50000) {
        uint64_t realTime = wifi.getRealTime();

        wifi.SendData("T " + String(realTime) + " W 2300 E 300 fr 0 fl 0 br 0 bl 0\n");
        lastSend = micros();
    }
}
