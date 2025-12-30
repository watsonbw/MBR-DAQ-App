#pragma once

#include <vector>

#include "data.hpp"

struct GLFWwindow;

class GUI {
public:
  GUI();
  ~GUI();

  void Launch();

private:
  void SetStyle();
  void DrawMainMenuBar();

private:
  GLFWwindow *m_Window;

  Data m_Data;

  float m_isConnected;

  bool m_IsLogging;
};