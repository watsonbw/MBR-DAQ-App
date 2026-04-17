// This is a skeleton for what the esp32 side of the wifi and collection should be
// When compiling, it is necessary to have both AsyncTCP and AsyncWebServer by ESP32 Async
// the string described below is how the app understands and interprets data
// it is required that the calculations are handled on the esp32 side

#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "board_wifi.hpp"
#include "rpm.hpp"
// #include "sd.hpp"

static const char*            ssid     = "esp32Wifi";
static const char*            password = "MBRdatacollect";
static BoardWifi              wifi{ssid, password};
static volatile unsigned long last_cleanup = 0;
static unsigned long          last_send    = 0;
volatile long                 test         = 0;
volatile bool                 last         = false;
volatile int                  wheel        = 0;
volatile int                  engine       = 0;
const int                     chipSelect   = 5;
volatile int                  test2        = 0;
RPMCollector                  wheel_rc;
RPMCollector                  engine_rc;

void setup() {
    Serial.begin(115200);
    pinMode(32, INPUT_PULLDOWN);
    pinMode(33, INPUT_PULLDOWN);

    wifi.Start();
    wifi.CleanupClients();

    if (!SD.begin(chipSelect)) {
        Serial.println("Card Mount Failed");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("No SD card attached");
        return;
    }
    // writeFile(SD, "/data.txt", "\n");
}

void loop() {

    wheel                    = wheel_rc.GetRPM(wifi.GetRealTime(), digitalRead(32)) / 2;
    engine                   = engine_rc.GetRPM(wifi.GetRealTime(), digitalRead(33));
    const uint64_t real_time = wifi.GetRealTime();

    if (millis() - last_cleanup > 10000) {
        wifi.CleanupClients();
        last_cleanup = millis();
    }
    if (real_time != 0) { test2 = simulateVehicleSpeed(); }
    if (micros() - last_send > 50000) {
        // wheel                    = wheel_rc.GetRPM(wifi.GetRealTime(), digitalRead(32));
        // engine                   = engine_rc.GetRPM(wifi.GetRealTime(), digitalRead(33));
        wifi.SendData("T " + String(real_time) + " W " + wheel + " E " + engine +
                      " fr 0 fl 0 br 0 bl 0\n");
        last_send = micros();
    }
}

void writeFile(fs::FS& fs, const char* path, const char* message) {
    File file = fs.open(path, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    if (file.print(message)) {
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

// Function to append data to a file
void appendFile(fs::FS& fs, const char* path, const char* message) {
    File file = fs.open(path, FILE_APPEND);
    if (!file) {
        Serial.println("Failed to open file for appending");
        return;
    }
    if (file.print(message)) {
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

int simulateVehicleSpeed() {
    static int speed     = 0;   // current speed
    static int direction = 1;   // 1 = increasing, -1 = decreasing
    const int  maxSpeed  = 255; // maximum speed
    const int  step      = 2;   // amount to change per call

    // update speed
    speed += step * direction;

    // flip direction at the max
    if (speed >= maxSpeed) {
        speed     = maxSpeed;
        direction = -1; // start decreasing
    }
    // stop when we reach 0
    else if (speed <= 0) {
        speed     = 0;
        direction = 0; // stop changing
    }

    return speed;
}
/*if (real_time != 0) {
            appendFile(SD,
                       "/data.txt",
                       ("T " + String(real_time) + " W " + String(wheel) + " E " + String(engine) +
                        " fr 0 fl 0 br 0 bl 0\n")
                           .c_str());
        }*/
