#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"

#include "app/context.hpp"
#include "app/pages/shock.hpp"

void ShockPage::OnEnter() { LOG_INFO("Entered ShockPage"); }
void ShockPage::OnExit() { LOG_INFO("Exitted ShockPage"); }

void ShockPage::Update() {
    const auto window_flags = DefaultWindowFlags();
    ImGui::Begin("Shock Data Collection", nullptr, window_flags);
    ImGui::Columns(2);

    // Left Side

    ImGui::Text("Data Log");
    ImGui::Separator();

    ImGui::BeginChild("Data Log Child");

    // Logging Button
    {
        ImGui::PushFont(m_Context->Fonts.Regular, 36.0f);

        bool logging = m_Context->IsLogging;
        if (ImGui::Button(logging ? "Stop Logging" : "Start Logging")) {
            m_Context->IsLogging = !logging;
        }

        ImGui::PopFont();
    }

    ImGui::Separator();

    std::vector<std::string> raw_data;
    std::vector<double>      time, fr, fl, br, bl;

    {
        std::lock_guard lock{m_Context->DataMutex};
        time = m_Context->Data.GetTime();

        const auto& shock_data = m_Context->Data.GetShockData();
        raw_data               = shock_data.RawLines;
        fr                     = shock_data.FrontRight;
        fl                     = shock_data.FrontLeft;
        br                     = shock_data.BackRight;
        bl                     = shock_data.BackRight;
    }

    for (const auto& msg : raw_data) {
        ImGui::TextUnformatted(msg.c_str());
    }

    ImGui::EndChild();

    // Right Side

    ImGui::NextColumn();

    ImGui::BeginChild("Graph Child");

    ImGui::Text("Shock Graph");
    ImGui::Separator();

    if (ImPlot::BeginPlot("RPM Over Time", {-1, -1})) {
        if (!time.empty()) {
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), fr.data(), time.size());
            }
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), fl.data(), time.size());
            }
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), br.data(), time.size());
            }
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), bl.data(), time.size());
            }
        }
        ImPlot::EndPlot();
    }

    ImGui::EndChild();

    ImGui::End();
}
