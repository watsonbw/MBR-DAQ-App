#pragma once

#include <string>
#include <vector>

struct AppContext;

struct RPMData {
    std::vector<double>      EngineRPM;
    std::vector<double>      WheelRPM;
    std::vector<std::string> RawLines;

    void Reserve(size_t size = 3000);
    void Clear();
};

struct ShockData {
    std::vector<double>      FrontRight;
    std::vector<double>      FrontLeft;
    std::vector<double>      BackRight;
    std::vector<double>      BackLeft;
    std::vector<std::string> RawLines;

    void Reserve(size_t size = 3000);
    void Clear();
};

class TelemetryData {
  public:
    explicit TelemetryData();
    ~TelemetryData() = default;

    const std::vector<double>& GetTime() const { return m_Time; }
    const RPMData&             GetRPMData() const { return m_RPMData; }
    const ShockData&           GetShockData() const { return m_ShockData; }
    void                       WriteData(std::string identifier, std::string value);

  private:
    void PopulateData(const char* esp32_data);

  private:
    std::vector<double> m_Time;
    RPMData             m_RPMData;
    ShockData           m_ShockData;
};
