#include <chrono>

#include <imgui.h>
#include <implot.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_log.h>

#include "app/gui.hpp"
#include "app/style.hpp"

#include "app/pages/home.hpp"
#include "app/pages/rpm.hpp"
#include "app/pages/shock.hpp"
#include "app/pages/view.hpp"

using namespace std::chrono;

static const char* WINDOW_TITLE = "Michigan Baja Racing - Data Suite";

static GUI* g_AppInstance = nullptr;

static void s_init() {
    if (g_AppInstance) { g_AppInstance->OnInit(); }
}
static void s_frame() {
    if (g_AppInstance) { g_AppInstance->OnFrame(); }
}
static void s_cleanup() {
    if (g_AppInstance) { g_AppInstance->OnCleanup(); }
}
static void s_event(const sapp_event* e) {
    if (g_AppInstance) { g_AppInstance->OnEvent(e); }
}

GUI::GUI(std::shared_ptr<AppContext> ctx) : m_Context{ctx} { }

sapp_desc GUI::GetSokolDesc() {
    g_AppInstance = this;

    sapp_desc desc          = {};
    desc.init_cb            = s_init;
    desc.frame_cb           = s_frame;
    desc.cleanup_cb         = s_cleanup;
    desc.event_cb           = s_event;
    desc.width              = 1920;
    desc.height             = 1080;
    desc.window_title       = WINDOW_TITLE;
    desc.icon.sokol_default = true;

    return desc;
}

void GUI::OnInit() {
    sg_desc sg_description     = {};
    sg_description.environment = sglue_environment();
    sg_description.logger.func = slog_func;
    sg_setup(&sg_description);

    simgui_desc_t si_desc = {};
    si_desc.no_default_font = true;
    simgui_setup(&si_desc);

    auto& io         = ImGui::GetIO();
    m_Context->Fonts = LoadFonts();
    io.FontDefault   = m_Context->Fonts.Regular;

    ImPlot::CreateContext();
    SetDarkThemeColors();
    
    ChangePage(PageType::HOME);
}

void GUI::OnFrame() {
    simgui_frame_desc_t frame_desc = {};
    frame_desc.width               = sapp_width();
    frame_desc.height              = sapp_height();
    frame_desc.delta_time          = sapp_frame_duration();
    frame_desc.dpi_scale           = sapp_dpi_scale();
    simgui_new_frame(&frame_desc);

    DrawMainMenuBar();

    if (m_CurrentPage) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        m_CurrentPage->Update();

        ImGui::PopStyleVar(2);
    }

    sg_pass_action pass_action        = {};
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value = {0.1f, 0.1f, 0.1f, 1.0f};

    sg_pass pass   = {};
    pass.action    = pass_action;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(&pass);
    simgui_render();
    sg_end_pass();
    sg_commit();
}

void GUI::OnEvent(const sapp_event* event) {
    // Forward events to ImGui
    simgui_handle_event(event);
}

void GUI::OnCleanup() {
    simgui_shutdown();
    sg_shutdown();
    ImPlot::DestroyContext();
}

void GUI::ChangePage(PageType type) {
    if (m_CurrentPage) { m_CurrentPage->OnExit(); }

    switch (type) {
    case PageType::HOME:
        m_CurrentPage = std::make_unique<HomePage>(m_Context);
        break;
    case PageType::RPM:
        m_CurrentPage = std::make_unique<RPMPage>(m_Context);
        break;
    case PageType::SHOCK:
        m_CurrentPage = std::make_unique<ShockPage>(m_Context);
        break;
    case PageType::VIEW:
        m_CurrentPage = std::make_unique<ViewPage>(m_Context);
        break;
    }

    if (m_CurrentPage) { m_CurrentPage->OnEnter(); }
    m_Context->CurrentPageType = type;
}

void GUI::DrawMainMenuBar() {
    ImGui::PushFont(m_Context->Fonts.Regular, 36.0f);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Home")) { ChangePage(PageType::HOME); }
            if (ImGui::MenuItem("RPM")) { ChangePage(PageType::RPM); }
            if (ImGui::MenuItem("Shock")) { ChangePage(PageType::SHOCK); }
            if (ImGui::MenuItem("View")) { ChangePage(PageType::VIEW); }
            if (ImGui::MenuItem("Exit")) { sapp_request_quit(); }

            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::TextUnformatted(PageTypeString(m_Context->CurrentPageType));

        ImGui::Separator();

        if (ImGui::Button("Sync Time")) {}

        ImGui::Separator();

        if (ImGui::Button("Restart Connection")) {}

        ImGui::Separator();

        auto now = system_clock::now();

        auto    time_now = system_clock::to_time_t(now);
        std::tm lt{};
#ifdef _WIN32
        localtime_s(&lt, &time_now);
#else
        localtime_r(&time_now, &lt);
#endif

        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        ImGui::Text("%02d:%02d:%02d.%03lld", lt.tm_hour, lt.tm_min, lt.tm_sec, ms.count());

        ImGui::EndMainMenuBar();
    }

    ImGui::PopFont();
}
