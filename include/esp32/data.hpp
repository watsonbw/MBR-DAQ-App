#pragma once

#include <vector>
#include <memory>
#include <thread>

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
    explicit TelemetryData(std::shared_ptr<AppContext> ctx);
    ~TelemetryData();

    void Start();
    
  private:
    void WorkerLoop();
    void PopulateData(const char* esp32_data);

  private:
    std::shared_ptr<AppContext> m_Context;
    std::thread m_Worker;
  
    RPMData   m_RPMData;
    ShockData m_ShockData;
    float     m_Time;
};
