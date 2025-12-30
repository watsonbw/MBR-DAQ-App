#pragma once

#include <vector>

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

class Data {
  public:
    Data();
    ~Data() = default;

    void PopulateData(const char* esp32_data);

  private:
    RPMData   m_RPMData;
    ShockData m_ShockData;
    float     m_Time;
};
