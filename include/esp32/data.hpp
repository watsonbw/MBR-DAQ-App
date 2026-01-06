#pragma once

#include <optional>
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
    struct PackedData {
        std::vector<uint64_t> TimeMicrosRaw;
        std::vector<double>   TimeMinutesNormalized;

        RPMData   RPM;
        ShockData Shock;

        std::vector<std::string> RawLines;
    };

  public:
    explicit TelemetryData();
    ~TelemetryData() = default;

    const std::vector<double>&      GetTime() const { return m_Time; }
    const std::vector<uint64_t>&    GetTimeNoNormal() const { return m_TimeNoNormalMicros; }
    const RPMData&                  GetRPMData() const { return m_RPMData; }
    const ShockData&                GetShockData() const { return m_ShockData; }
    const std::vector<std::string>& GetRawLines() const { return m_RawLines; };
    const std::optional<LocalTime>& GetSyncLT() const { return m_SyncLT; }

    void WriteData(const std::string& identifier, const std::string& value);
    void WriteRawLine(const std::string& message);
    void Clear();

  private:
    std::vector<uint64_t>    m_TimeNoNormalMicros;
    std::vector<double>      m_Time;
    RPMData                  m_RPMData;
    ShockData                m_ShockData;
    std::vector<std::string> m_RawLines;
    double                   m_SyncStart;
    std::optional<LocalTime> m_SyncLT;

    friend class ViewPage;
};
