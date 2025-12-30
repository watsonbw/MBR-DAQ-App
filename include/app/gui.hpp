#pragma once

#include <memory>

#include "app/context.hpp"
#include "app/pages/page.hpp"

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

    void ChangePage(PageType type);

    void StartFrame();
    void Update();
    void EndFrame();

    void DrawMainMenuBar();

  private:
    GLFWwindow* m_Window;
    WindowData  m_WindowData;

    std::unique_ptr<Page>       m_CurrentPage;
    std::shared_ptr<AppContext> m_Context;
};
