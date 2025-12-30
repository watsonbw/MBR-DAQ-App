#pragma once

#include <vector>

struct AppContext;

struct RPMData {
    std::vector<float> EngineRPM;
    std::vector<float> WheelRPM;

    void Reserve(size_t size = 3000);
    void Clear();
};

struct ShockData {
    std::vector<float> FrontRight;
    std::vector<float> FrontLeft;
    std::vector<float> BackRight;
    std::vector<float> BackLeft;

    void Reserve(size_t size = 3000);
    void Clear();
};

class TelemetryData {
  public:
    explicit TelemetryData();
    ~TelemetryData() = default;

  private:
    void PopulateData(const char* esp32_data);

  private:
    RPMData   m_RPMData;
    ShockData m_ShockData;
    float     m_Time;
};
