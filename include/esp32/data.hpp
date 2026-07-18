#pragma once

#include <optional>
#include <string>
#include <vector>

#include "core/time.hpp"
#include <nlohmann/json.hpp>

#define MBR_JSON "MBR_data.json"

namespace mbr {

struct app_context;

using json = nlohmann::json;

class TelemetryData {
  public:
    struct PackedData {
        std::vector<uint64_t>                                TimeMicrosRaw;
        std::vector<double>                                  TimeMinutesNormalized;
        std::unordered_map<std::string, std::vector<double>> Series;
        std::vector<std::string>                             RawLines;
    };

    struct DataInfo {
        std::string Key;
        std::string Name;
        bool        Required = false;
        bool        Plot     = false;
        std::string Unit;
        std::string Group;
    };

  public:
    explicit TelemetryData() {
        InitJSON();
        InitData();
    }
    ~TelemetryData() = default;

    std::vector<DataInfo>                                DataValues;
    std::unordered_map<std::string, std::vector<double>> Series;
    [[nodiscard]] const std::vector<double>&             GetTime() const { return m_Time; }
    [[nodiscard]] const std::vector<uint64_t>&           GetTimeNoNormal() const {
        return m_TimeNoNormalMicros;
    }

    [[nodiscard]] const std::vector<std::string>& GetRawLines() const { return m_RawLines; };
    [[nodiscard]] const std::optional<local_time>& GetSyncLT() const { return m_SyncLT; }
    [[nodiscard]] const std::string&              GetCurrentLine() { return m_CurrentLine; }

    void WriteData(const std::string& identifier, const std::string& value);
    void WriteRawLine(const std::string& message);
    void SaveCurrentLine(const std::string& line);
    void Clear();

  private:
    void                     InitJSON();
    void                     InitData();
    std::string              m_CurrentLine;
    std::vector<uint64_t>    m_TimeNoNormalMicros;
    std::vector<double>      m_Time;
    std::vector<std::string> m_RawLines;
    double                   m_SyncStart;
    std::optional<local_time> m_SyncLT;

    friend class ViewPage;
};

} // namespace mbr
