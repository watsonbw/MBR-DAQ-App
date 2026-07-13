#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "core/time.hpp"

struct AppContext;

using json = nlohmann::json;

class TelemetryData {
  public:
    struct PackedData {
        std::vector<uint64_t> TimeMicrosRaw;
        std::vector<double>   TimeMinutesNormalized;

        std::vector<std::string> RawLines;
    };


    struct DataConfig {
        std::string Key;
        std::string Name;
        std::string Type;
        bool Required = false;
        bool Plot     = false;
        std::string Unit;
        std::string Group;
    };

  public:
    explicit TelemetryData();
    ~TelemetryData() = default;

    [[nodiscard]] const std::vector<double>&   GetTime() const { return m_Time; }
    [[nodiscard]] const std::vector<uint64_t>& GetTimeNoNormal() const {
        return m_TimeNoNormalMicros;
    }

    [[nodiscard]] const std::vector<std::string>& GetRawLines() const { return m_RawLines; };
    [[nodiscard]] const std::optional<LocalTime>& GetSyncLT() const { return m_SyncLT; }
    [[nodiscard]] const std::string&              GetCurrentLine() { return m_CurrentLine; }

    void WriteData(const std::string& identifier, const std::string& value);
    void WriteRawLine(const std::string& message);
    void SaveCurrentLine(const std::string& line);
    void Clear();

  private:

    [[nodiscard]] bool InitJSON(std::vector<DataConfig>& out) const;
    std::unordered_map<std::string, std::vector<double>> m_Series;
    std::vector<DataConfig> m_DataValues;
    std::string              m_CurrentLine;
    std::vector<uint64_t>    m_TimeNoNormalMicros;
    std::vector<double>      m_Time;
    std::vector<std::string> m_RawLines;
    double                   m_SyncStart;
    std::optional<LocalTime> m_SyncLT;

    friend class ViewPage;
};
