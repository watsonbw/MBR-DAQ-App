#include <cassert>
#include <sstream>

#include <imgui.h>
#include <implot.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_log.h>

#include "core/time.hpp"

#include "app/assets/images/app_icon.hpp"

#include "app/gui.hpp"
#include "app/style.hpp"

#include "app/pages/home.hpp"
#include "app/pages/rpm.hpp"
#include "app/pages/shock.hpp"
#include "app/pages/view.hpp"

using namespace std::chrono;

GUI* GUI::s_Instance = nullptr;

#define SAFE_SOKOL_CB(F) \
    assert(s_Instance);  \
    s_Instance->F

void GUI::SokolInitCB() { SAFE_SOKOL_CB(OnInit()); }
void GUI::SokolCleanupCB() { SAFE_SOKOL_CB(OnCleanup()); }
void GUI::SokolFrameCB() { SAFE_SOKOL_CB(OnFrame()); }
void GUI::SokolEventCB(const sapp_event* e) { SAFE_SOKOL_CB(OnEvent(e)); }

sapp_desc GUI::GetSokolDesc() {
    assert(s_Instance == nullptr);
    s_Instance = this;

    sapp_desc desc    = {};
    desc.init_cb      = GUI::SokolInitCB;
    desc.frame_cb     = GUI::SokolFrameCB;
    desc.cleanup_cb   = GUI::SokolCleanupCB;
    desc.event_cb     = GUI::SokolEventCB;
    desc.width        = 1920;
    desc.height       = 1080;
    desc.window_title = "Michigan Baja Racing - Data Suite";

    desc.icon.sokol_default    = false;
    m_AppIcon                  = {BajaLogo_png, BajaLogo_png_size};
    desc.icon.images[0].width  = m_AppIcon.Width;
    desc.icon.images[0].height = m_AppIcon.Height;
    desc.icon.images[0].pixels = {m_AppIcon.Pixels, m_AppIcon.Size};

    return desc;
}

void GUI::OnInit() {
    sg_desc sg_description     = {};
    sg_description.environment = sglue_environment();
    sg_description.logger.func = slog_func;
    sg_setup(&sg_description);

    simgui_desc_t si_desc   = {};
    si_desc.no_default_font = true;
    simgui_setup(&si_desc);

    auto& io                      = ImGui::GetIO();
    m_Context->Style.DefaultFonts = LoadFonts();
    io.FontDefault                = m_Context->Style.DefaultFonts.Regular;

    ImPlot::CreateContext();
    m_Context->Style.SetDarkThemeColors();

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

    if (m_Context->Backend->TryConnection == true) {
        m_Context->Backend->Start();
        m_Context->Backend->TryConnection = false;
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
    if (event->type == SAPP_EVENTTYPE_QUIT_REQUESTED) { sapp_quit(); }
    simgui_handle_event(event);
}

void GUI::OnCleanup() {
    m_Context->ShouldExit = true;
    if (m_CurrentPage) { m_CurrentPage->OnExit(); }
    m_CurrentPage.reset();

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
    ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("Menu")) {
            ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 28.0f);
            
            if (ImGui::MenuItem("Home")) { ChangePage(PageType::HOME); }
            if (ImGui::MenuItem("RPM")) { ChangePage(PageType::RPM); }
            if (ImGui::MenuItem("Shock")) { ChangePage(PageType::SHOCK); }
            if (ImGui::MenuItem("View")) { ChangePage(PageType::VIEW); }
            if (ImGui::MenuItem("Toggle Dark Mode")) {
                if (m_Context->Style.DarkMode) {
                    m_Context->Style.SetLightThemeColors();
                } else {
                    m_Context->Style.SetDarkThemeColors();
                }
            }
            if (ImGui::MenuItem("Exit")) { sapp_request_quit(); }
            
            ImGui::PopFont();
            ImGui::EndMenu();
        }

        ImGui::Separator();

        ImGui::TextUnformatted(PageTypeString(m_Context->CurrentPageType));

        ImGui::Separator();

        LocalTime lt;
        auto      sync_time = lt.MicrosSinceMidnight();

        std::stringstream ss;
        ss << "SYNC ";
        ss << sync_time;
        if (ImGui::Button("Sync Time")) { m_Context->Backend->SendCMD(ss.str()); }

        ImGui::Separator();

        if (ImGui::Button("Restart Connection")) { m_Context->Backend->TryConnection = true; }

        ImGui::Separator();

        if (ImGui::Button("Clear Data")) { m_Context->Backend->Data.Clear(); }

        ImGui::Separator();

        // Connection indicator

        bool connected = m_Context->Backend->IsConnected;
        bool receiving = m_Context->Backend->IsReceiving;

        float  radius = 10.0f;
        ImVec2 pos    = ImGui::GetCursorScreenPos();
        ImVec2 center = ImVec2(pos.x + radius, pos.y + ImGui::GetFrameHeight() * 0.5f);

        ImU32 color = connected && receiving    ? IM_COL32(0, 200, 0, 255)
                      : connected && !receiving ? IM_COL32(255, 255, 0, 255)
                                                : IM_COL32(200, 0, 0, 255);

        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddCircleFilled(center, radius, color);

        ImGui::Dummy(ImVec2(radius * 2.5f, radius * 2.0f));
        ImGui::SameLine();
        ImGui::TextUnformatted(connected ? "Connected" : "Not Connected");

        ImGui::Separator();

        static char buffer[128] = "";
        ImGui::SetNextItemWidth(200.0f);
        ImGui::InputText("##SmallBox", buffer, 128);

        ImGui::Separator();

        if (ImGui::Button("Send CMD")) {
            m_Context->Backend->SendCMD(buffer);
            buffer[0] = '\0';
        }

        ImGui::Separator();

        const std::string time_formatted = lt.String();
        ImGui::TextUnformatted(time_formatted.c_str());

        ImGui::EndMainMenuBar();
    }

    ImGui::PopFont();
}
