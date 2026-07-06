#include "sd.hpp"
#include "SD.h"

SDCard::SDCard() = default;

bool SDCard::OpenSD(const char* name) {

    File check  = SD.open(name, FILE_READ);
    bool exists = static_cast<bool>(check);
    if (check) { check.close(); }

    if (exists) {
        m_LogFile = SD.open(name, FILE_APPEND);
    } else {
        m_LogFile = SD.open(name, FILE_WRITE);
    }

    if (m_LogFile) {
        IsOpen = true;
        if (exists) {
            Serial.println("SD File re-opened successfully. Streaming active.");
        } else {
            Serial.println("SD File opened successfully. Streaming active.");
        }
        return true;
    }
    Serial.println("Failed to open file for appending.");
    return false;
}

void SDCard::WriteSD(const char* msg) {
    if (!IsOpen || !m_LogFile) { return; }

    m_LogFile.print(msg);

    if (millis() - m_LastFlush > 5000) {
        m_LogFile.flush();
        m_LastFlush = millis();
    }
}

bool SDCard::CloseSD() {
    if (IsOpen && m_LogFile) {
        m_LogFile.close();
        IsOpen  = false;
        IsWrite = false;
        Serial.println("SD File cleanly closed.");
        return true;
    }
    return false;
}

bool SDCard::InitSD() const {
    if (!SD.begin(m_Chipselect)) {
        Serial.println("SD mount fail");
        return false;
    }
    Serial.println("SD mounted");
    return true;
}
