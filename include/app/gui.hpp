#pragma once

#include <memory>

#include "assets/texture.hpp"

#include "app/context.hpp"
#include "app/pages/page.hpp"

struct sapp_event;
struct sapp_desc;

class GUI {
  public:
    static void SokolInitCB();
    static void SokolCleanupCB();
    static void SokolFrameCB();
    static void SokolEventCB(const sapp_event* e);

  public:
    explicit GUI(const std::shared_ptr<AppContext>& ctx) : m_Context{ctx} {};
    ~GUI() = default;

    sapp_desc GetSokolDesc();

    void OnInit();
    void OnFrame();
    void OnEvent(const sapp_event* event);
    void OnCleanup();

    static void SokolStartFrame();
    static void SokolEndFrame();

  private:
    void ChangePage(PageType type);

    void DrawMainMenuBar();

  private:
    static GUI* s_Instance; // NOLINT

    std::unique_ptr<Page>       m_CurrentPage;
    std::shared_ptr<AppContext> m_Context;
    IconTexture                 m_AppIcon;
    std::string                 m_CommandBuf;
};
