#ifndef MBR_CAN_H
#define MBR_CAN_H

#include <Arduino.h>

// Struct to keep CAN messages unified across platforms
struct MbrCanMessage {
    uint32_t Id;
    bool Extended;
    uint8_t Length;
    uint8_t Data[8];
};

void InitCAN();
bool SendCANMessage(const MbrCanMessage& msg);
bool ReadCANMessage(MbrCanMessage& msg);

#endif