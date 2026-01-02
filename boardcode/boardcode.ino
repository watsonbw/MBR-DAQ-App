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
  // put your main code here, to run repeatedly:
  Serial.println("this thing is on");
  delay(1000);
}
