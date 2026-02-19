#include "rpm.hpp"
#include <cstdint>

RPMCollector::RPMCollector() {
    // Initialize any variables here if needed, e.g.:
    // last_micros = 0;
}

auto RPMCollector::GetRPM(uint32_t timestamp, uint8_t pin_value) -> double {
    if (timestamp == 0) { return 0.0; }
    if (pin_value == HI && timestamp > Thresh()) {
        m_RPM    = 0.0;
        m_WasRPM = 0;
        // Do something meaningful maybe
    } else if (pin_value == LO && !m_WasRPM) {
        const auto elapsed_us = timestamp - m_LastTime;
        if (elapsed_us > 1000) {
            m_RPM      = 60000000.0 / elapsed_us;
            m_LastTime = timestamp;
            m_WasRPM   = 1;
        }

    } else if (pin_value == HI) {
        m_WasRPM = 0;
    }
    return m_RPM;
}
