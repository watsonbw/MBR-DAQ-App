#include "sd.hpp"

SDCard::SDCard() : isOpen(false) {}

bool SDCard::OpenSD(const char* name) {

    File check  = SD.open(name, FILE_READ);
    bool exists = (bool)check;
    if (check) check.close();

    if (exists) {
        logFile = SD.open(name, FILE_APPEND);
    } else {
        logFile = SD.open(name, FILE_WRITE);
    }

    if (logFile) {
        isOpen = 1;
        if (exists) {
            Serial.println("SD File re-opened successfully. Streaming active.");
        } else {
            Serial.println("SD File opened successfully. Streaming active.");
        }
        return true;
    } else {
        Serial.println("Failed to open file for appending.");
        return false;
    }
}

void SDCard::WriteSD(const char* msg) {
    if (!isOpen || !logFile) { return; }

    logFile.print(msg);

    if (millis() - lastFlush > 5000) {
        logFile.flush();
        lastFlush = millis();
    }
}

bool SDCard::CloseSD() {
    if (isOpen && logFile) {
        logFile.close();
        isOpen  = false;
        isWrite = false;
        Serial.println("SD File cleanly closed.");
        return 1;
    } else {
        return 0;
    }
}

bool SDCard::InitSD() {
    if (!SD.begin(chipselect)) {
        Serial.println("SD mount fail");
        return false;
    }
    Serial.println("SD mounted");
    return true;
}
