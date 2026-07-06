#include "canbus_backend.hpp"

#if defined(ARDUINO_ARCH_ESP32)
    // Use the native high-performance ESP32 library
    #include <ESP32-TWAI-CAN.hpp>
    #define CAN_RX_PIN 4
    #define CAN_TX_PIN 5
#elif defined(ARDUINO_ARCH_STM32)
    // Use a native STM32 CAN library (like STM32_CAN)
    #include <STM32_CAN.h>
#endif

void InitCAN() {
    #if defined(ARDUINO_ARCH_ESP32)
        // Setup native ESP32 TWAI hardware
        ESP32Can.setPins(CAN_TX_PIN, CAN_RX_PIN);
        ESP32Can.begin(TWAI_SPEED_500KBPS);
    #elif defined(ARDUINO_ARCH_STM32)
        // Setup native STM32 hardware
        STM32Can.begin(500000);
    #endif
}

bool SendCANMessage(const MbrCanMessage& msg) {
    #if defined(ARDUINO_ARCH_ESP32)
        // Convert MbrCanMessage to ESP32 TWAI format and send

        return true;
    #elif defined(ARDUINO_ARCH_STM32)
        // Convert MbrCanMessage to STM32 format and send

        return true;
    #endif
}

bool ReceiveCANMessage(){
    #if defined(ARDUINO_ARCH_ESP32)
    // Interpret CAN message and translate to ESP32 format

        return true;
    #elif defined(ARDUINO_ARCH_STM32)
    // Interpret CAN message and translate to STM format

        return true;
    #endif
}
