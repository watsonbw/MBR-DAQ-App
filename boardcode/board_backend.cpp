#include "board_backend.hpp"
#include <Arduino.h>

int64_t BoardBackend::m_LastSend = 0;
int64_t BoardBackend::m_LastCleanup = 0;


BoardBackend::BoardBackend(const char* ssid, const char* password) 
    : m_SSID(ssid), m_Password(password), m_wifi(ssid, password){};


void BoardBackend::Initialize(){ 
    m_wifi.Start();
    CleanupClients();
    m_sd.OpenSD();
    //canbus.Start(); I believe
}

//Runs the backend
void BoardBackend::Run(){
    const uint64_t real_time = GetRealTime();
    if ((uint32_t)(millis()) - m_LastCleanup > 10000) {
        CleanupClients();
        m_LastCleanup = millis();
    }

    //collect data here, will be changed with canbus implementation
    m_wheel                    = wheel_rc.GetRPM(micros(), digitalRead(32)) / 2; 
    m_engine                   = engine_rc.GetRPM(micros(), digitalRead(33));

    snprintf(m_Msg, sizeof(m_Msg), "T %llu W %d E %d\n", GetRealTime(), m_wheel, m_engine);

    if ((uint32_t)(esp_timer_get_time()) - m_LastSend > 50000) {
        SendData(m_Msg);
        m_sd.WriteSD(m_Msg);
        m_LastSend = micros();
    }

    if (m_wifi.m_NewCommand || 0 /*add for LoRa behavior*/){
        ReceiveData();
    }
}

uint64_t BoardBackend::GetRealTime() {
    if (!m_IsTimeSynced) { return 0; 
    } else {
        int64_t elapsedMicros = (uint32_t)esp_timer_get_time() - m_LocalSyncMicros;
        return (uint64_t)m_BaseTimeMicros + (uint64_t)(elapsedMicros);
    }
}

void BoardBackend::SendData(const char* msg){
    m_wifi.SendData(msg);
}

void BoardBackend::ReceiveData(){
            if (m_WifiOn){
            if (strncmp(m_wifi.m_CommandValue, "SYNC", 4) == 0) {
                const char* timeStr = m_wifi.m_CommandValue + 4;
                m_LocalSyncMicros = esp_timer_get_time();
                m_BaseTimeMicros  = strtoull(timeStr, NULL, 10);
                m_IsTimeSynced    = 1;
            } else {
                /*currently does nothing, need to work on other command implementation*/
            }
        }

        if (m_LoRaOn /*currently its never on */){

        }
        m_wifi.m_NewCommand = false;
}
