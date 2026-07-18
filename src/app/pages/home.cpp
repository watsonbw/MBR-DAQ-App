#include <fmt/format.h>
#include <gsl/util>
#include <imgui.h>
#include <sokol_app.h>

#include "core/log.hpp"

#include "app/pages/home.hpp"
#include "app/pages/utils.hpp"
#include "app/style.hpp"

namespace mbr {

void HomePage::OnEnter() { LOG_INFO("Entered HomePage"); }
void HomePage::OnExit() { LOG_INFO("Exited HomePage"); }

void HomePage::Update() {
    const float full_height   = ImGui::GetContentRegionAvail().y;
    const float top_height    = full_height * 0.66F;
    const float bottom_height = full_height - top_height;

    if (ImGui::BeginChild("##topsec", {0, top_height})) {
        const auto cleanup_top{gsl::finally(ImGui::EndChild)};
        if (ImGui::BeginTable("##topsplt", 2, ImGuiTableFlags_BordersInnerV)) {
            const auto cleanup_split{gsl::finally(ImGui::EndTable)};
            ImGui::TableNextColumn();
            DrawTopLHS();
            ImGui::TableNextColumn();
            DrawTopRHS();
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
            DrawBottomLHS();
            ImGui::TableNextColumn();
            DrawBottomRHS();
        }
    }
}

void HomePage::DrawTopLHS() {
    BOLD_HEADER(ImGui::Text("SD Card Control"));
    ImGui::Separator();

    if (ImGui::BeginChild("##sd_control", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        local_time  t;
        HEADER(pages::utils::draw_input_box(
            "##sd_name", m_SetName, fmt::format("Name ({})", t.to_string(false)).c_str()));
        ImGui::SameLine();
        if (ImGui::Button("Create File")) {
            if (m_SetName.empty()) {
                m_SDName = t.to_string(false);
            } else {
                m_SDName = m_SetName;
            }
            for (char& c : m_SDName) {
                if (c == ':' || c == ' ') { c = '-'; }
            }
            const auto command = fmt::format("SD_START /{}.txt", m_SDName);
            context_->backend->SendCMD(command);
            m_SetName.clear();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button,
                              context_->backend->IsWriting ? ImVec4(0, 0.7F, 0, 1)
                                                            : ImVec4(0.7F, 0, 0, 1));
        if (ImGui::Button(context_->backend->IsWriting ? "Write ON" : "Write OFF")) {
            if (context_->backend->IsWriting) {
                context_->backend->SendCMD("SD_WRITE 0");
            } else {
                context_->backend->SendCMD("SD_WRITE 1");
            }
            // m_SDWrite = !m_SDWrite;
        }
        ImGui::PopStyleColor();
        ImGui::SameLine();
        if (!m_SDName.empty()) {
            if (ImGui::Button(context_->backend->IsOpen ? "Close SD" : "Open SD")) {
                if (context_->backend->IsOpen) {
                    context_->backend->SendCMD("SD_CLOSE");
                } else {
                    const auto command = fmt::format("SD_START /{}.txt", m_SDName);
                    context_->backend->SendCMD(command);
                }
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("%s", !m_SDName.empty() ? m_SDName.c_str() : "No file created");
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

void HomePage::DrawTopRHS() {
    BOLD_HEADER(ImGui::Text("Control Panel"));
    ImGui::Separator();

    BOLD_DEFAULT(ImGui::SeparatorText("Metrics"));
    ImGui::BulletText("Backend Status: %s", context_->backend->IsConnected ? "Online" : "Offline");
    ImGui::BulletText("Last IP Update: %s", m_PreviousIp.to_string().c_str());
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

void HomePage::DrawBottomLHS() {
    BOLD_HEADER(ImGui::Text("Error Log"));
    ImGui::Separator();

    if (ImGui::BeginChild("##errscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto        cleanup{gsl::finally(ImGui::EndChild)};
        const std::string all_errors = Log::GetStreamedLogs();
        ImGui::TextUnformatted(all_errors.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
    }
}

void HomePage::DrawBottomRHS() {
    BOLD_HEADER(ImGui::Text("Command Center"));
    ImGui::Separator();

    if (ImGui::BeginChild("##commandcenter", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        DrawIPControls();
        DrawCredentialControls();
    }
}

void HomePage::DrawIPControls() {
    if (ImGui::Button("Update IP") && !m_IpBuf.any_empty()) {
        context_->backend->SetIp(m_IpBuf);
        m_PreviousIp = std::exchange(m_IpBuf, {});
    }

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_FIRST", m_IpBuf.first, m_PreviousIp.first.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_SECOND", m_IpBuf.second, m_PreviousIp.second.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_THIRD", m_IpBuf.third, m_PreviousIp.third.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_FOURTH", m_IpBuf.fourth, m_PreviousIp.fourth.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(":");

    ImGui::SameLine();
    pages::utils::draw_input_box("##ip_PORT", m_IpBuf.port, m_PreviousIp.port.c_str(), 50.0F);
}

void HomePage::DrawCredentialControls() {
    // TODO(blake): Do something with user/password
    if (ImGui::Button("Upload Credentials") && (!m_UsernameBuf.empty() && !m_PasswordBuf.empty())) {
        m_UsernameBuf = {};
        m_PasswordBuf = {};
    }

    ImGui::SameLine();
    pages::utils::draw_input_box("##username", m_UsernameBuf, "Username", 150.0F);
    ImGui::SameLine();
    pages::utils::draw_input_box("##password", m_PasswordBuf, "Password", 150.0F);
}

} // namespace mbr
