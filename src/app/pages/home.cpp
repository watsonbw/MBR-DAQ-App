#include "app/pages/home.hpp"

#include <string>
#include <utility>

#include <fmt/format.h>
#include <gsl/util>
#include <imgui.h>
#include <sokol_app.h>

#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

namespace mbr::pages {

void home_page::on_enter() { log_info(context_->log, "Entered HomePage"); }
void home_page::on_exit() { log_info(context_->log, "Exited HomePage"); }

void home_page::update() {
    const float full_height   = ImGui::GetContentRegionAvail().y;
    const float top_height    = full_height * 0.66F;
    const float bottom_height = full_height - top_height;

    if (ImGui::BeginChild("##topsec", {0, top_height})) {
        const auto cleanup_top{gsl::finally(ImGui::EndChild)};
        if (ImGui::BeginTable("##topsplt", 2, ImGuiTableFlags_BordersInnerV)) {
            const auto cleanup_split{gsl::finally(ImGui::EndTable)};
            ImGui::TableNextColumn();
            draw_top_lhs();
            ImGui::TableNextColumn();
            draw_top_rhs();
        }
    }
    ImGui::Separator();

    if (ImGui::BeginChild("##botsec", {0, bottom_height})) {
        const auto cleanup_bot{gsl::finally(ImGui::EndChild)};
        if (ImGui::BeginTable("##botsplt", 2, ImGuiTableFlags_Resizable)) {
            const auto cleanup_split{gsl::finally(ImGui::EndTable)};
            ImGui::TableSetupColumn("##errors", ImGuiTableColumnFlags_WidthStretch, 0.33F);
            ImGui::TableSetupColumn("##action", ImGuiTableColumnFlags_WidthStretch, 0.66F);

            ImGui::TableNextColumn();
            draw_bottom_lhs();
            ImGui::TableNextColumn();
            draw_bottom_rhs();
        }
    }
}

void home_page::draw_top_lhs() {
    BOLD_HEADER(ImGui::Text("SD Card Control"));
    ImGui::Separator();

    if (ImGui::BeginChild("##sd_control", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        local_time t;
        HEADER(pages::utils::draw_input_box(
            "##sd_name", set_name_, fmt::format("Name ({})", t.to_string(false)).c_str()));
        ImGui::SameLine();
        if (ImGui::Button("Create File")) {
            if (set_name_.empty()) {
                sd_name_ = t.to_string(false);
            } else {
                sd_name_ = set_name_;
            }
            for (char& c : sd_name_) {
                if (c == ':' || c == ' ') { c = '-'; }
            }
            const auto command = fmt::format("SD_START /{}.txt", sd_name_);
            context_->backend->send_cmd(command);
            set_name_.clear();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,
                              context_->backend->is_writing ? ImVec4(0, 0.7F, 0, 1)
                                                            : ImVec4(0.7F, 0, 0, 1));
        if (ImGui::Button(context_->backend->is_writing ? "Write ON" : "Write OFF")) {
            if (context_->backend->is_writing) {
                context_->backend->send_cmd("SD_WRITE 0");
            } else {
                context_->backend->send_cmd("SD_WRITE 1");
            }
            // m_SDWrite = !m_SDWrite;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (!sd_name_.empty()) {
            if (ImGui::Button(context_->backend->is_open ? "Close SD" : "Open SD")) {
                if (context_->backend->is_open) {
                    context_->backend->send_cmd("SD_CLOSE");
                } else {
                    const auto command = fmt::format("SD_START /{}.txt", sd_name_);
                    context_->backend->send_cmd(command);
                }
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", !sd_name_.empty() ? sd_name_.c_str() : "No file created");
        /*
        ImGui::BulletText("Connect to Wifi on laptop");
        ImGui::BulletText(
        "Make sure you see 'Connected' and a green light, that means you are receiving data");
        ImGui::BulletText("If not, restart everything: app, esp32, wifi");
        ImGui::BulletText("Once connection is established, press 'Sync Time', this will allow the "
        "data to be displayed");
        ImGui::BulletText("No data will be collected unless time is synced");
        ImGui::BulletText("You must sync time everytime the esp32 is restarted");
        ImGui::BulletText("Under the 'Menu' dropdown there are various pages");
        ImGui::BulletText("Go to the page you want data from");
        ImGui::BulletText("Press 'Start Logging'");
        ImGui::BulletText("Once you've gathered the data, you can download it by just pressing "
        "download, or giving it a title");
        ImGui::BulletText("No title just gives the time stamp");
        ImGui::BulletText(
        "In the 'View' page, you can upload data and video to watch the data be plotted live");
        ImGui::BulletText("First upload both the video and the data file");
        ImGui::BulletText(
        "Then type in the timestamp (down to the second) of when the video was created");
        ImGui::BulletText("Press 'Sync Data/Video'");
        ImGui::BulletText("Turn on dynamic plotting and watch the data be plotted");
        ImGui::BulletText(
        "You can also hide data you don't want to see by pressing on them in the legend");
        ImGui::BulletText("Send CMD currently works, but there aren't any commands supported");
            */
    }
}

void home_page::draw_top_rhs() {
    BOLD_HEADER(ImGui::Text("Control Panel"));
    ImGui::Separator();

    BOLD_DEFAULT(ImGui::SeparatorText("Metrics"));
    ImGui::BulletText("Backend Status: %s", context_->backend->is_connected ? "Online" : "Offline");
    ImGui::BulletText("Last IP Update: %s", previous_ip_.to_string().c_str());
    ImGui::BulletText("Application FPS: %.2f", ImGui::GetIO().Framerate);

    BOLD_DEFAULT(ImGui::SeparatorText("UI Settings"));
    bool dark_mode = context_->style.dark_mode;
    if (ImGui::Checkbox("Dark Mode", &dark_mode)) {
        if (dark_mode) {
            context_->style.set_dark_theme();
        } else {
            context_->style.set_light_theme();
        }
    }

    bool fullscreen = sapp_is_fullscreen();
    if (ImGui::Checkbox("Fullscreen", &fullscreen)) { sapp_toggle_fullscreen(); }
}

void home_page::draw_bottom_lhs() {
    BOLD_HEADER(ImGui::Text("Error Log"));
    ImGui::Separator();

    if (ImGui::BeginChild("##errscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto        cleanup{gsl::finally(ImGui::EndChild)};
        const std::string all_errors = context_->logger.get_streamed_logs();
        ImGui::TextUnformatted(all_errors.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
    }
}

void home_page::draw_bottom_rhs() {
    BOLD_HEADER(ImGui::Text("Command Center"));
    ImGui::Separator();

    if (ImGui::BeginChild("##commandcenter", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        draw_ip_controls();
        draw_credential_controls();
    }
}

void home_page::draw_ip_controls() {
    if (ImGui::Button("Update IP") && !ip_buf_.any_empty()) {
        context_->backend->set_ip(ip_buf_);
        previous_ip_ = std::exchange(ip_buf_, {});
    }

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_FIRST", ip_buf_.first, previous_ip_.first.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_SECOND", ip_buf_.second, previous_ip_.second.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_THIRD", ip_buf_.third, previous_ip_.third.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_FOURTH", ip_buf_.fourth, previous_ip_.fourth.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(":");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_PORT", ip_buf_.port, previous_ip_.port.c_str(), 50.0F);
}

void home_page::draw_credential_controls() {
    // TODO(blake): Do something with user/password
    if (ImGui::Button("Upload Credentials") && (!username_buf_.empty() && !password_buf_.empty())) {
        username_buf_ = {};
        password_buf_ = {};
    }

    ImGui::SameLine();
    pages::utils::draw_input_box("##username", username_buf_, "Username", 150.0F);
    ImGui::SameLine();
    pages::utils::draw_input_box("##password", password_buf_, "Password", 150.0F);
}

} // namespace mbr::pages
