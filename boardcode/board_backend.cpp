#include "board_backend.hpp"
#include <Arduino.h>

int64_t BoardBackend::m_LastSend    = 0;
int64_t BoardBackend::m_LastCleanup = 0;

BoardBackend::BoardBackend(const char* ssid, const char* password)
    : m_SSID(ssid), m_Password(password), m_Wifi(ssid, password) {};

void BoardBackend::Initialize() {
    m_Wifi.Start();
    CleanupClients();
    m_Sd.InitSD();
    // canbus.Start(); I believe
}

// Runs the backend
void BoardBackend::Run() {
    int64_t now = esp_timer_get_time();
    if (now - m_LastCleanup > 10000000LL) {
        CleanupClients();
        m_LastCleanup = now;
    }

    // collect data here, will be changed with canbus implementation
    WheelRPM  = m_WheelRC.GetRPM(esp_timer_get_time(), digitalRead(32)) / 2;
    EngineRPM = m_EngineRC.GetRPM(esp_timer_get_time(), digitalRead(33));

    snprintf(m_Msg, sizeof(m_Msg), "T %llu W %f E %f\n", GetRealTime(), WheelRPM, EngineRPM);

    if ((now - m_LastSend > 50000LL) && m_IsTimeSynced) {
        SendData(m_Msg);
        if (m_Sd.IsOpen && m_Sd.IsWrite) { m_Sd.WriteSD(m_Msg); }
        m_LastSend = now;
    }

    if (m_Wifi.NewCommand /*add for LoRa behavior*/) { ReceiveData(); }
}

uint64_t BoardBackend::GetRealTime() const {
    if (!m_IsTimeSynced) {
        return 0;
    }

    int64_t current_time = esp_timer_get_time();
    auto elapsed_micros = static_cast<uint64_t>(current_time - m_LocalSyncMicros);
    return static_cast<uint64_t>(m_BaseTimeMicros) + elapsed_micros;
}

void BoardBackend::SendData(const char* msg) { m_Wifi.SendData(msg); }

void BoardBackend::ReceiveData() {
    m_Wifi.NewCommand = false;
    char res[64];
    if (m_WifiOn) {
        if (strncmp(m_Wifi.CommandValue, "SYNC", 4) == 0) {
            const char* time_str = m_Wifi.CommandValue + 4;
            m_LocalSyncMicros   = esp_timer_get_time();
            m_BaseTimeMicros    = static_cast<int64_t>(strtoull(time_str, nullptr, 10));
            m_IsTimeSynced      = true;
            if (m_BaseTimeMicros != 0) {
                snprintf(res, sizeof(res), "RES 0 SYNC %lld\n", m_BaseTimeMicros);
                Serial.println(res);
                SendData(res);
            }
        } else if (strncmp(m_Wifi.CommandValue, "SD_START", 8) == 0) {
            const char* name_str = m_Wifi.CommandValue + 9;
            if (*name_str == '\0') { name_str = "/data.txt"; }
            if (m_FileCount < MAX_FILES) {
                bool exists = false;
                for (int i = 0; i < m_FileCount; i++) {
                    if (strncmp(m_FileIndex[i], name_str, MAX_NAME_LEN) == 0) {
                        exists = true;
                        break;
                    }
                }
                if (!exists) { strncpy(m_FileIndex[m_FileCount++], name_str, MAX_NAME_LEN - 1); }
            } else {
                SendData("RES 0 SD_START deadbeef\n");
                return;
            }
            if (m_Sd.IsOpen) {
                if (m_Sd.CloseSD()) {
                    SendData("RES 0 SD_WRITE 0\n");
                    SendData("RES 0 SD_CLOSE 1\n");
                } else {
                    SendData("RES 0 SD_CLOSE 0\n");
                }
            }
            if (!m_Sd.OpenSD(name_str)) {
                SendData("RES 0 SD_START deadbeef\n");
                return;
            }
            m_Sd.Name = name_str;
            snprintf(res, sizeof(res), "RES 0 SD_START %s\n", name_str);
            SendData(res);
        } else if (strncmp(m_Wifi.CommandValue, "SD_WRITE", 8) == 0) {
            const char* value_str = m_Wifi.CommandValue + 9;
            m_Sd.IsWrite         = (*value_str == '1');
            snprintf(res, sizeof(res), "RES 0 SD_WRITE %d\n", m_Sd.IsWrite);
            SendData(res);
        } else if (strncmp(m_Wifi.CommandValue, "SD_CLOSE", 8) == 0) {
            if (m_Sd.CloseSD()) {
                SendData("RES 0 SD_WRITE 0\n");
                SendData("RES 0 SD_CLOSE 1\n");
            } else {
                SendData("RES 0 SD_CLOSE 0\n");
            }
        } else if (strncmp(m_Wifi.CommandValue, "STATUS", 6) == 0) {
            for (size_t i = 0; i < MAX_FILES; i++) {
                // snprintf(res, sizeof(res), "RES 0 SD_OPEN %d\n", m_FileNames[i]);
                // SendData(res);
            }
        } else {
            /*currently does nothing, need to work on other command implementation*/
            SendData("RES 1\n");
        }
    }

    if (m_LoRaOn /*currently its never on */) {}
}
