#pragma once

#include <memory>

#include "app/context.hpp"
#include "app/pages/page.hpp"

struct sapp_event;
struct sapp_desc;

struct WindowData {
    int DisplayWidth;
    int DisplayHeight;
};

class GUI {
  public:
    explicit GUI(std::shared_ptr<AppContext> ctx);
    ~GUI() = default;

    sapp_desc GetSokolDesc();
    
    void OnInit();
    void OnFrame();
    void OnEvent(const sapp_event* event);
    void OnCleanup();

  private:
    void ChangePage(PageType type);

    void DrawMainMenuBar();

  private:
    std::unique_ptr<Page>       m_CurrentPage;
    std::shared_ptr<AppContext> m_Context;
};
