#pragma once

#include <vector>

struct RPMData {
  std::vector<float> engine_RPM;
  std::vector<float> wheel_RPM;
};

struct ShockData {
  std::vector<float> front_Right;
  std::vector<float> front_Left;
  std::vector<float> back_Right;
  std::vector<float> back_Left;
};
