#include "sd.hpp"

SDCard::SDCard() : isOpen(false) {}

void SDCard::OpenSD(){
    if (!SD.begin(chipselect)){
        Serial.println("SD mount fail");
        return;
    }

    logFile = SD.open("/data.txt", FILE_APPEND);
    if (logFile){
        isOpen = 1;
        Serial.println("SD File opened successfully. Streaming active.");
    } else {
        Serial.println("Failed to open file for appending.");
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

void SDCard::CloseSD(){
    if (isOpen && logFile) {
        logFile.close();
        isOpen = false;
        Serial.println("SD File cleanly closed.");
    }
}