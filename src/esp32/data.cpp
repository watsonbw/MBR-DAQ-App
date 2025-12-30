#include "esp32/data.hpp"

void RPMData::Reserve(size_t size) {
    EngineRPM.reserve(size);
    WheelRPM.reserve(size);
}

void RPMData::Clear() {
    EngineRPM.clear();
    WheelRPM.clear();
}

void ShockData::Reserve(size_t size) {
    FrontRight.reserve(size);
    FrontLeft.reserve(size);
    BackRight.reserve(size);
    BackLeft.reserve(size);
}

void ShockData::Clear() {
    FrontRight.clear();
    FrontLeft.clear();
    BackRight.clear();
    BackLeft.clear();
}

Data::Data() : m_Time{0.0f} {
    m_RPMData.Reserve();
    m_ShockData.Reserve();

    // Make this do something if you add more member variables
}

void Data::PopulateData([[maybe_unused]] const char* esp32_data) {
    // Empty the data buffers but retain their capacity
    m_RPMData.Clear();
    m_ShockData.Clear();

    // Do something fun with the raw bytes to parse
}
