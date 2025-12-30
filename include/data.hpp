#pragma once

#include <vector>

struct RPMData {
  std::vector<float> engine_RPM;
  std::vector<float> wheel_RPM;

  inline void reserve(size_t size = 1000) {
    engine_RPM.reserve(size);
    wheel_RPM.reserve(size);
  }

  inline void clear() {
    engine_RPM.clear();
    wheel_RPM.clear();
  }
};

struct ShockData {
  std::vector<float> front_Right;
  std::vector<float> front_Left;
  std::vector<float> back_Right;
  std::vector<float> back_Left;

  inline void reserve(size_t size = 1000) {
    front_Right.reserve(size);
    front_Left.reserve(size);
    back_Right.reserve(size);
    back_Left.reserve(size);
  }

  inline void clear() {
    front_Right.clear();
    front_Left.clear();
    back_Right.clear();
    back_Left.clear();
  }
};

class Data {
public:
  Data();
  ~Data() = default;

  void populate_data(const char *esp32_data);

private:
  RPMData m_RPMData;
  ShockData m_ShockData;
  float m_Time;
};
