#pragma once

#include <memory>

#include "app/context.hpp"
#include "app/style.hpp"

struct GLFWwindow;

struct WindowData {
    int DisplayWidth;
    int DisplayHeight;
};

class GUI {
  public:
    explicit GUI(std::shared_ptr<AppContext> ctx);
    ~GUI();

    void Launch();

  private:
    bool InitGLFW();
    void InitImGui();

    void StartFrame();
    void Update();
    void EndFrame();

    void DrawMainMenuBar();
    void DrawDashboard();

  private:
    GLFWwindow* m_Window;
    WindowData  m_WindowData;

    AppFonts m_Fonts;

    std::shared_ptr<AppContext> m_Context;
};
