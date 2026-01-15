#include <imgui.h>

#include <sokol_app.h>

#include "core/log.hpp"

#include "app/common/scope.hpp"
#include "app/common/text.hpp"
#include "app/pages/home.hpp"
#include "app/style.hpp"

void HomePage::OnEnter() { LOG_INFO("Entered HomePage"); }
void HomePage::OnExit() { LOG_INFO("Exited HomePage"); }

void HomePage::Update() {
    const float full_height   = ImGui::GetContentRegionAvail().y;
    const float top_height    = full_height * 0.66F;
    const float bottom_height = full_height - top_height;

    if (const ImGuiScope<ImGui::EndChild> top_section{
            IMSCOPE_FN(ImGui::BeginChild("##topsec", {0, top_height}))}) {
        if (const ImGuiScope<ImGui::EndTable> split{
                IMSCOPE_FN(ImGui::BeginTable("##topsplt", 2, ImGuiTableFlags_BordersInnerV))}) {
            ImGui::TableNextColumn();
            DrawTopLHS();
            ImGui::TableNextColumn();
            DrawTopRHS();
        }
    }
    ImGui::Separator();

    if (const ImGuiScope<ImGui::EndChild> bottom_section{
            IMSCOPE_FN(ImGui::BeginChild("##botsec", {0, bottom_height}))}) {
        if (const ImGuiScope<ImGui::EndTable> split{
                IMSCOPE_FN(ImGui::BeginTable("##botsplt", 2, ImGuiTableFlags_Resizable))}) {
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
    BOLD_HEADER(ImGui::Text("How To"));
    ImGui::Separator();

    if (const ImGuiScope<ImGui::EndChild> tutorial{IMSCOPE_FN(ImGui::BeginChild(
            "##tutorial", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar))}) {
        ImGui::BulletText("Connect to Wifi on laptop");
        ImGui::BulletText("Make sure you see 'Connected' and a green light, that means you are receiving data");
        ImGui::BulletText("If not, restart everything: app, esp32, wifi");
        ImGui::BulletText("Once connection is established, press 'Sync Time', this will allow the data to be displayed");
        ImGui::BulletText("No data will be collected unless time is synced");
        ImGui::BulletText("You must sync time everytime the esp32 is restarted");
        ImGui::BulletText("Under the 'Menu' dropdown there are various pages");
        ImGui::BulletText("Go to the page you want data from");
        ImGui::BulletText("Press 'Start Logging'");
        ImGui::BulletText("Once you've gathered the data, you can download it by just pressing download, or giving it a title");
        ImGui::BulletText("No title just gives the time stamp");
        ImGui::BulletText("In the 'View' page, you can upload data and video to watch the data be plotted live");
        ImGui::BulletText("First upload both the video and the data file");
        ImGui::BulletText("Then type in the timestamp (down to the second) of when the video was created");
        ImGui::BulletText("Press 'Sync Data/Video'");
        ImGui::BulletText("Turn on dynamic plotting and watch the data be plotted");
        ImGui::BulletText("You can also hide data you don't want to see by pressing on them in the legend");
        ImGui::BulletText("Send CMD currently works, but there aren't any commands supported");
    }
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

    if (const ImGuiScope<ImGui::EndChild> scroll{IMSCOPE_FN(ImGui::BeginChild(
            "##errscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar))}) {
        const std::string all_errors = Log::GetStreamedLogs();
        ImGui::TextUnformatted(all_errors.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
    }
}

void HomePage::DrawBottomRHS() {
    BOLD_HEADER(ImGui::Text("Command Center"));
    ImGui::Separator();

    if (const ImGuiScope<ImGui::EndChild> command_center{IMSCOPE_FN(ImGui::BeginChild(
            "##commandcenter", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar))}) {
        DrawIPControls();
        DrawCredentialControls();
    }
}

void HomePage::DrawIPControls() {
    if (ImGui::Button("Update IP") && !m_IpBuf.AnyEmpty()) {
        m_Context->Backend->SetIp(m_IpBuf);
        m_PreviousIp = std::exchange(m_IpBuf, {});
    }

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_FIRST", m_IpBuf.First, m_PreviousIp.First.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_SECOND", m_IpBuf.Second, m_PreviousIp.Second.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_THIRD", m_IpBuf.Third, m_PreviousIp.Third.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(".");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_FOURTH", m_IpBuf.Fourth, m_PreviousIp.Fourth.c_str(), 75.0F);
    ImGui::SameLine();
    ImGui::TextUnformatted(":");

    ImGui::SameLine();
    TextUtils::DrawInputBox("##ip_PORT", m_IpBuf.Port, m_PreviousIp.Port.c_str(), 50.0F);
}

void HomePage::DrawCredentialControls() {
    // TODO(blake): Do something with user/password
    if (ImGui::Button("Upload Credentials") && (!m_UsernameBuf.empty() && !m_PasswordBuf.empty())) {
        m_UsernameBuf = {};
        m_PasswordBuf = {};
    }

    ImGui::SameLine();
    TextUtils::DrawInputBox("##username", m_UsernameBuf, "Username", 150.0F);
    ImGui::SameLine();
    TextUtils::DrawInputBox("##password", m_PasswordBuf, "Password", 150.0F);
}
