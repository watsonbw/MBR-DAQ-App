// This is a skeleton for what the esp32 side of the wifi and collection should be
// When compiling, it is necessary to have both AsyncTCP and AsyncWebServer by ESP32 Async
// the string described below is how the app understands and interprets data
// it is required that the calculations are handled on the esp32 side

#include "board_backend.hpp"

static const char*  ssid     = "esp32Wifi";
static const char*  password = "MBRdatacollect";
static BoardBackend backend{ssid, password};

void setup() {
    Serial.begin(115200);
    pinMode(32, INPUT_PULLDOWN);
    pinMode(33, INPUT_PULLDOWN);
    backend.Initialize();
}

void loop() { backend.Run(); }
