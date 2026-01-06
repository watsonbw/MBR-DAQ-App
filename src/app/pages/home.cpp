#include <imgui.h>

#include <sokol_app.h>

#include "core/log.hpp"

#include "app/pages/common/text.hpp"
#include "app/pages/home.hpp"
#include "app/style.hpp"

void HomePage::OnEnter() { LOG_INFO("Entered HomePage"); }
void HomePage::OnExit() { LOG_INFO("Exited HomePage"); }

void HomePage::Update() {
    const float full_height   = ImGui::GetContentRegionAvail().y;
    const float top_height    = full_height * 0.66f;
    const float bottom_height = full_height - top_height;

    if (ImGui::BeginChild("TopSection", {0, top_height})) {
        if (ImGui::BeginTable("TopSplit", 2, ImGuiTableFlags_BordersInnerV)) {
            ImGui::TableNextColumn();
            DrawTopLHS();
            ImGui::TableNextColumn();
            DrawTopRHS();

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
    ImGui::Separator();

    if (ImGui::BeginChild("BottomSection", {0, bottom_height})) {
        if (ImGui::BeginTable("BottomSplit", 2, ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Errors", ImGuiTableColumnFlags_WidthStretch, 0.33f);
            ImGui::TableSetupColumn("ActionPanel", ImGuiTableColumnFlags_WidthStretch, 0.66f);

            ImGui::TableNextColumn();
            DrawBottomLHS();
            ImGui::TableNextColumn();
            DrawBottomRHS();

            ImGui::EndTable();
        }
    }
    ImGui::EndChild();
}

void HomePage::DrawTopLHS() {
    BOLD_HEADER(ImGui::Text("How To"));
    ImGui::Separator();

    if (ImGui::BeginChild("Tutorial", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        ImGui::BulletText("Touch my tail?");
        ImGui::BulletText("Uhm....");
        ImGui::BulletText("No!");
    }
    ImGui::EndChild();
}

void HomePage::DrawTopRHS() {
    BOLD_HEADER(ImGui::Text("Control Panel"));
    ImGui::Separator();

    BOLD_DEFAULT(ImGui::SeparatorText("Metrics"));
    ImGui::BulletText("Backend Status: %s", m_Context->Backend->IsConnected ? "Online" : "Offline");
    ImGui::BulletText("Last IP Update: %s", m_PreviousIp.String().c_str());
    ImGui::BulletText("Application FPS: %.2f", ImGui::GetIO().Framerate);

    BOLD_DEFAULT(ImGui::SeparatorText("UI Settings"));
    bool dark_mode = m_Context->Style.DarkMode;
    if (ImGui::Checkbox("Dark Mode", &dark_mode)) {
        if (dark_mode) {
            m_Context->Style.SetDarkThemeColors();
        } else {
            m_Context->Style.SetLightThemeColors();
        }
    }

    bool fullscreen = sapp_is_fullscreen();
    if (ImGui::Checkbox("Fullscreen", &fullscreen)) { sapp_toggle_fullscreen(); }
}

void HomePage::DrawBottomLHS() {
    BOLD_HEADER(ImGui::Text("Error Log"));
    ImGui::Separator();

    if (ImGui::BeginChild(
            "ErrorScrollingRegion", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const std::string all_errors = Log::GetStreamedLogs();
        ImGui::TextUnformatted(all_errors.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0f); }
    }
    ImGui::EndChild();
}

void HomePage::DrawBottomRHS() {
    BOLD_HEADER(ImGui::Text("Command Center"));
    ImGui::Separator();

    if (ImGui::BeginChild("CommandCenter", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        DrawIPControls();
        DrawCredentialControls();
    }
    ImGui::EndChild();
}

void HomePage::DrawIPControls() {
    if (ImGui::Button("Update IP") && !m_IpBuf.AnyEmpty()) {
        m_Context->Backend->SetIp(m_IpBuf);
        m_PreviousIp = std::exchange(m_IpBuf, {});
    }

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_FIRST", m_IpBuf.First, m_PreviousIp.First.c_str(), 75.0f);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_SECOND", m_IpBuf.Second, m_PreviousIp.Second.c_str(), 75.0f);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_THIRD", m_IpBuf.Third, m_PreviousIp.Third.c_str(), 75.0f);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_FOURTH", m_IpBuf.Fourth, m_PreviousIp.Fourth.c_str(), 75.0f);
    ImGui::SameLine();
    ImGui::TextUnformatted(":");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_PORT", m_IpBuf.Port, m_PreviousIp.Port.c_str(), 50.0f);
}

void HomePage::DrawCredentialControls() {
    // TODO: Do something with user/password
    if (ImGui::Button("Upload Credentials") && (!m_UsernameBuf.empty() && !m_PasswordBuf.empty())) {
        m_UsernameBuf = {};
        m_PasswordBuf = {};
    }

    ImGui::SameLine();
    TextUtils::DrawInputBox("##username", m_UsernameBuf, "Username", 150.0f);
    ImGui::SameLine();
    TextUtils::DrawInputBox("##password", m_PasswordBuf, "Password", 150.0f);
}
