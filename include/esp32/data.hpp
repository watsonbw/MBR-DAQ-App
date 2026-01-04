#pragma once

#include <string>
#include <vector>
#include "core/time.hpp"

struct AppContext;

struct RPMData {
    std::vector<double> EngineRPM;
    std::vector<double> WheelRPM;

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

    const std::vector<double>&      GetTime() const { return m_Time; }
    const RPMData&                  GetRPMData() const { return m_RPMData; }
    const ShockData&                GetShockData() const { return m_ShockData; }
    const std::vector<std::string>& GetRawLines() const { return m_RawLines; };
    void                            WriteData(std::string identifier, std::string value);
    void                            WriteRawLine(const std::string& message);
    void                            Clear();
    bool                            IsSynced{false};
    LocalTime                       SyncLT;

  private:
    std::vector<double>      m_Time;
    RPMData                  m_RPMData;
    ShockData                m_ShockData;
    std::vector<std::string> m_RawLines;
    double                   m_SyncStart;
};
