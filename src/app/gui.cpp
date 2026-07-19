#include "app/gui.hpp"

#include <memory>
#include <string>

#include <fmt/format.h>
#include <gsl/pointers>
#include <gsl/util>
#include <imgui.h>
#include <implot.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <sokol_log.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/assets/images/app_icon.hpp"
#include "app/assets/texture.hpp"
#include "app/context.hpp"
#include "app/pages/home.hpp"
#include "app/pages/rpm.hpp"
#include "app/pages/serialmon.hpp"
#include "app/pages/shock.hpp"
#include "app/pages/utils.hpp"
#include "app/pages/view.hpp"
#include "app/style.hpp"
#include "core/time.hpp"

using namespace std::chrono;

namespace mbr {

namespace {

void sokol_init_cb(void* data) {
    gsl::not_null gui = static_cast<gui_t*>(data);
    gui->on_init();
}

void sokol_cleanup_cb(void* data) {
    gsl::not_null gui = static_cast<gui_t*>(data);
    gui->on_cleanup();
}

void sokol_frame_cb(void* data) {
    gsl::not_null gui = static_cast<gui_t*>(data);
    gui->on_frame();
}

void sokol_event_cb(const sapp_event* e, void* data) {
    gsl::not_null gui = static_cast<gui_t*>(data);
    gui->on_event(e);
}

void sokol_start_frame() {
    simgui_frame_desc_t frame_desc = {};
    frame_desc.width               = sapp_width();
    frame_desc.height              = sapp_height();
    frame_desc.delta_time          = sapp_frame_duration();
    frame_desc.dpi_scale           = sapp_dpi_scale();
    simgui_new_frame(&frame_desc);
}

void sokol_end_frame() {
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

} // namespace

sapp_desc gui_t::get_sokol_desc() {
    sapp_desc desc           = {};
    desc.init_userdata_cb    = sokol_init_cb;
    desc.frame_userdata_cb   = sokol_frame_cb;
    desc.cleanup_userdata_cb = sokol_cleanup_cb;
    desc.event_userdata_cb   = sokol_event_cb;
    desc.width               = 1'920;
    desc.height              = 1'080;
    desc.window_title        = "Michigan Baja Racing - Data Suite";
    desc.icon.sokol_default  = false;
    desc.user_data           = this;

    // The app description takes ownership of the icon here
    assets::icon_texture<true> app_icon{assets::BAJA_LOGO_PNG};
    desc.icon.images[0].width  = app_icon.width;
    desc.icon.images[0].height = app_icon.height;
    desc.icon.images[0].pixels = {.ptr = app_icon.pixels, .size = app_icon.size};

    return desc;
}

void gui_t::on_init() {
    sg_desc sg_description     = {};
    sg_description.environment = sglue_environment();
    sg_description.logger.func = slog_func;
    sg_setup(&sg_description);

    simgui_desc_t si_desc   = {};
    si_desc.no_default_font = true;
    simgui_setup(&si_desc);

    auto& io                      = ImGui::GetIO();
    context_->style.default_fonts = load_fonts();
    io.FontDefault                = context_->style.default_fonts.regular;

    ImPlot::CreateContext();
    context_->style.set_dark_theme();

    change_page(page_type_t::HOME);
}

void gui_t::on_frame() {
    PROFILE_FUNCTION();
    sokol_start_frame();
    const auto cleanup_frame{gsl::finally(sokol_end_frame)};
    MAIN_MENU_BAR(draw_main_menu_bar());

    if (current_page_) {
        const auto* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0F);
        const auto sv1{gsl::finally([] { ImGui::PopStyleVar(1); })};
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0F);
        const auto sv2{gsl::finally([] { ImGui::PopStyleVar(1); })};

        static constexpr ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        if (ImGui::Begin("##currpage", nullptr, window_flags)) {
            const auto cleanup_page{gsl::finally(ImGui::End)};
            current_page_->update();
        }
    }

    if (ImGui::IsKeyPressed(ImGuiKey_F11, false)) { sapp_toggle_fullscreen(); }
    if (context_->backend->get_try_connection().exchange(false)) { context_->backend->start(); }
}

void gui_t::on_event(const sapp_event* event) { // NOLINT
    PROFILE_FUNCTION();
    if (event->type == SAPP_EVENTTYPE_QUIT_REQUESTED) { sapp_quit(); }
    simgui_handle_event(event);
}

void gui_t::on_cleanup() {
    context_->should_exit = true;
    if (current_page_) { current_page_->on_exit(); }
    current_page_.reset();

    simgui_shutdown();
    sg_shutdown();
    ImPlot::DestroyContext();
}

void gui_t::change_page(page_type_t type) {
    if (current_page_) { current_page_->on_exit(); }

    switch (type) {
    case page_type_t::HOME:   current_page_ = std::make_unique<pages::home_page>(context_); break;
    case page_type_t::RPM:    current_page_ = std::make_unique<pages::rpm_page>(context_); break;
    case page_type_t::SHOCK:  current_page_ = std::make_unique<pages::shock_page>(context_); break;
    case page_type_t::VIEW:   current_page_ = std::make_unique<pages::view_page>(context_); break;
    case page_type_t::SERIAL: current_page_ = std::make_unique<pages::serial_page>(context_); break;
    }

    if (current_page_) { current_page_->on_enter(); }
    context_->current_page_type = type;
}

void gui_t::draw_main_menu_bar() {
    PROFILE_FUNCTION();
    // DO NOT MOVE THIS BEGIN CALL IT WILL BREAK
    if (ImGui::BeginMainMenuBar()) {
        const auto cleanup_mm{gsl::finally(ImGui::EndMainMenuBar)};
        if (ImGui::BeginMenu("Menu")) {
            const auto cleanup_m{gsl::finally(ImGui::EndMenu)};
            MAIN_MENU_BAR_ITEM({
                if (ImGui::MenuItem("Home")) { change_page(page_type_t::HOME); }
                if (ImGui::MenuItem("RPM")) { change_page(page_type_t::RPM); }
                if (ImGui::MenuItem("Shock")) { change_page(page_type_t::SHOCK); }
                if (ImGui::MenuItem("View")) { change_page(page_type_t::VIEW); }
                if (ImGui::MenuItem("Serial Monitor")) { change_page(page_type_t::SERIAL); }
                if (ImGui::MenuItem("Toggle Dark Mode")) {
                    if (context_->style.dark_mode) {
                        context_->style.set_light_theme();
                    } else {
                        context_->style.set_dark_theme();
                    }
                }
                if (ImGui::MenuItem("Exit")) { sapp_request_quit(); }
            });
        }

        ImGui::Separator();
        ImGui::TextUnformatted(page_type_str(context_->current_page_type));
        ImGui::Separator();
        // ImGui::TextUnformatted(page_type_str(m_Context->CurrentPageType));
        // ImGui::Separator();

        const local_time lt;
        const auto       sync_time = lt.micros_since_midnight();

        const auto command = fmt::format("CMD SYNC {}", sync_time);
        if (ImGui::Button("Sync Time")) { context_->backend->send_cmd(command); }

        ImGui::Separator();
        if (ImGui::Button("Restart Connection")) { context_->backend->get_try_connection() = true; }
        ImGui::Separator();
        if (ImGui::Button("Clear Data")) { context_->backend->get_data().clear(); }
        ImGui::Separator();

        // Connection indicator
        const bool connected = context_->backend->is_connected();
        const bool receiving = context_->backend->is_receiving();

        const f32    radius = 10.0F;
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
        pages::utils::draw_input_box("##command", command_buf_);
        context_->is_cmd_input_focused = ImGui::IsItemFocused();
        ImGui::Separator();

        if (ImGui::Button("Send CMD")) {
            context_->backend->send_cmd(command_buf_);
            command_buf_ = {};
        }

        ImGui::Separator();

        const std::string time_formatted = lt.to_string();
        ImGui::TextUnformatted(time_formatted.c_str());
    }
}

} // namespace mbr
