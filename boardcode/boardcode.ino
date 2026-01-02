#include "board_wifi.hpp"

const char* ssid = "esp32Wifi";
const char* password = "MBRdatacollect";
BoardWifi wifi(ssid, password);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  wifi.Start();
}

void loop() {

    wifi.cleanupClients(); 


    static unsigned long lastSend = 0;
    if (micros() - lastSend > 50) {
      uint64_t realTime = wifi.getRealTime();

        wifi.SendData("T " + String(realTime) + " W 2300 E 300 fr 0 fl 0 br 0 bl 0");
        lastSend = micros();
    }
}
