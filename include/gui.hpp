#pragma once

#include <vector>

#include "data.hpp"

struct GLFWwindow;

class GUI {
public:
  GUI();
  ~GUI();

  void launch();

private:
  GLFWwindow *m_Window;

  Data m_Data;

  float m_isConnected;

  // Example - REMOVE ME
  float m_RPM;
  std::vector<float> m_RPMHistory;
  bool m_IsLogging;
};