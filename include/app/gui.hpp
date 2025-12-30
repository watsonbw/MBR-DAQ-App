#pragma once

#include "esp32/data.hpp"

struct GLFWwindow;

struct WindowData {
    int DisplayWidth;
    int DisplayHeight;
};

class GUI {
  public:
    GUI();
    ~GUI();

    void Launch();

  private:
    bool InitGLFW();
    void InitImGUI();

    void StartFrame();
    void EndFrame();

    void DrawMainMenuBar();
    void DrawDashboard();

  private:
    GLFWwindow* m_Window;
    WindowData  m_WindowData;

    Data  m_Data;
    float m_IsConnected;
    bool  m_IsLogging;
};
