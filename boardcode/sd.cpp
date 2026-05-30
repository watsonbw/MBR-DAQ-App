#include "sd.hpp"

SDCard::SDCard() : isOpen(false) {}

bool SDCard::OpenSD(const char* name){
    if (!SD.begin(chipselect)){
        Serial.println("SD mount fail");
        return false;
    }

    logFile = SD.open(name, FILE_APPEND);
    if (logFile){
        isOpen = 1;
        Serial.println("SD File opened successfully. Streaming active.");
        return true;
    } else {
        Serial.println("Failed to open file for appending.");
        return false;
    }

}

void SDCard::WriteSD(const char* msg){
    if (!isOpen || !logFile){
       return;
    }

    logFile.print(msg);

    if (millis() - lastFlush > 5000){
        logFile.flush();
        lastFlush = millis();
    }
}

bool SDCard::CloseSD(){
    if (isOpen && logFile) {
        logFile.close();
        isOpen = false;
        isWrite = false;
        Serial.println("SD File cleanly closed.");
        return 1;
    } else {
        return 0;
    }
}