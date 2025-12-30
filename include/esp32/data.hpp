#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct AppContext;

struct RPMData {
    std::vector<double>       EngineRPM;
    std::vector<double>       WheelRPM;
    std::vector<std::string> RawLines;

    void Reserve(size_t size = 3000);
    void Clear();
};

struct ShockData {
    std::vector<double> FrontRight;
    std::vector<double> FrontLeft;
    std::vector<double> BackRight;
    std::vector<double> BackLeft;

    void Reserve(size_t size = 3000);
    void Clear();
};

class TelemetryData {
  public:
    explicit TelemetryData();
    ~TelemetryData() = default;

    const std::vector<double>& getTime() const { return m_Time; }
    const RPMData&               getRPMData() const { return m_RPMData; }
    const ShockData&             getShockData() const { return m_ShockData; }

  private:
    void PopulateData(const char* esp32_data);

  private:
    std::vector<double> m_Time;
    RPMData               m_RPMData;
    ShockData             m_ShockData;
};
