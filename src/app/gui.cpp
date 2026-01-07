#include <cassert>

#include <imgui.h>
#include <implot.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_log.h>

#include "core/time.hpp"

#include "app/assets/images/app_icon.hpp"

#include "app/common/scope.hpp"
#include "app/common/text.hpp"
#include "app/gui.hpp"
#include "app/style.hpp"

#include "app/pages/home.hpp"
#include "app/pages/rpm.hpp"
#include "app/pages/shock.hpp"
#include "app/pages/view.hpp"

using namespace std::chrono;

GUI* GUI::s_Instance = nullptr;

#define SOKOL_CB(F)     \
    assert(s_Instance); \
    s_Instance->F

void GUI::SokolInitCB() { SOKOL_CB(OnInit()); }
void GUI::SokolCleanupCB() { SOKOL_CB(OnCleanup()); }
void GUI::SokolFrameCB() { SOKOL_CB(OnFrame()); }
void GUI::SokolEventCB(const sapp_event* e) { SOKOL_CB(OnEvent(e)); }

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
    m_AppIcon                  = {BAJA_LOGO_PNG, BAJA_LOGO_PNG_SIZE};
    desc.icon.images[0].width  = m_AppIcon.Width;
    desc.icon.images[0].height = m_AppIcon.Height;
    desc.icon.images[0].pixels = {.ptr = m_AppIcon.Pixels, .size = m_AppIcon.Size};

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
    const RenderScope<SokolEndFrame> frame{SokolStartFrame};
    MAIN_MENU_BAR(DrawMainMenuBar());

    if (m_CurrentPage) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        const ImGuiScope<ImGui::PopStyleVar, 1> sv1{
            IMSCOPE_FN(ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F))};
        const ImGuiScope<ImGui::PopStyleVar, 1> sv2{
            IMSCOPE_FN(ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F))};

        constexpr ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        if (const ImGuiScope<ImGui::End> page{
                IMSCOPE_FN(ImGui::Begin("##currpage", nullptr, window_flags))}) {
            m_CurrentPage->Update();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) { sapp_toggle_fullscreen(); }
    if (m_Context->Backend->TryConnection.exchange(false)) { m_Context->Backend->Start(); }
}

void GUI::OnEvent(const sapp_event* event) { // NOLINT
    switch (event->type) {
    case SAPP_EVENTTYPE_QUIT_REQUESTED:
        sapp_quit();
        break;
    default:
        break;
    }
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

void GUI::SokolStartFrame() {
    simgui_frame_desc_t frame_desc = {};
    frame_desc.width               = sapp_width();
    frame_desc.height              = sapp_height();
    frame_desc.delta_time          = sapp_frame_duration();
    frame_desc.dpi_scale           = sapp_dpi_scale();
    simgui_new_frame(&frame_desc);
}

void GUI::SokolEndFrame() {
    sg_pass_action pass_action        = {};
    pass_action.colors[0].load_action = SG_LOADACTION_CLEAR;
    pass_action.colors[0].clear_value = {.r = 0.1F, .g = 0.1F, .b = 0.1F, .a = 1.0F};

    sg_pass pass   = {};
    pass.action    = pass_action;
    pass.swapchain = sglue_swapchain();

    sg_begin_pass(&pass);
    simgui_render();
    sg_end_pass();
    sg_commit();
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
    // DO NOT MOVE THIS BEGIN CALL IT WILL BREAK
    if (const ImGuiScope<ImGui::EndMainMenuBar, REQUIRE_ALIVE_FOR_DTOR> main_menu_bar{
            IMSCOPE_FN(ImGui::BeginMainMenuBar())}) {
        if (const ImGuiScope<ImGui::EndMenu, REQUIRE_ALIVE_FOR_DTOR> menu{
                IMSCOPE_FN(ImGui::BeginMenu("Menu"))}) {
            MAIN_MENU_BAR_ITEM({
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
            });
        }

        ImGui::Separator();
        ImGui::TextUnformatted(PageTypeString(m_Context->CurrentPageType));
        ImGui::Separator();
        ImGui::TextUnformatted(PageTypeString(m_Context->CurrentPageType));
        ImGui::Separator();

        const LocalTime lt;
        const auto      sync_time = lt.MicrosSinceMidnight();

        const auto command = std::format("SYNC {}", sync_time);
        if (ImGui::Button("Sync Time")) { m_Context->Backend->SendCMD(command); }

        ImGui::Separator();
        if (ImGui::Button("Restart Connection")) { m_Context->Backend->TryConnection = true; }
        ImGui::Separator();
        if (ImGui::Button("Clear Data")) { m_Context->Backend->Data.Clear(); }
        ImGui::Separator();

        // Connection indicator
        const bool connected = m_Context->Backend->IsConnected;
        const bool receiving = m_Context->Backend->IsReceiving;

        const float  radius = 10.0F;
        const ImVec2 pos    = ImGui::GetCursorScreenPos();
        const ImVec2 center = ImVec2(pos.x + radius, pos.y + (ImGui::GetFrameHeight() * 0.5F));

        ImU32 color;
        if (connected) {
            color = receiving ? IM_COL32(0, 200, 0, 255) : IM_COL32(255, 255, 0, 255);
        } else {
            color = IM_COL32(200, 0, 0, 255);
        }

        ImDrawList* draw = ImGui::GetWindowDrawList();
        draw->AddCircleFilled(center, radius, color);

        ImGui::Dummy({radius * 2.5F, radius * 2.0F});
        ImGui::SameLine();
        ImGui::TextUnformatted(connected ? "Connected" : "Not Connected");

        ImGui::Separator();
        TextUtils::DrawInputBox("##command", m_CommandBuf);
        m_Context->CommandInputFocused = ImGui::IsItemFocused();
        ImGui::Separator();

        if (ImGui::Button("Send CMD")) {
            m_Context->Backend->SendCMD(m_CommandBuf);
            m_CommandBuf = {};
        }

        ImGui::Separator();

        const std::string time_formatted = lt.String();
        ImGui::TextUnformatted(time_formatted.c_str());
    }
}
