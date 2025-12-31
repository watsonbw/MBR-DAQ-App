#include <iostream>

#include <imgui.h>
#include <implot.h>

#include "app/context.hpp"
#include "app/pages/rpm.hpp"

void RPMPage::OnEnter() { std::cout << "[INFO]: Entered RPMPage" << "\n"; }
void RPMPage::OnExit() { std::cout << "[INFO]: Exitted RPMPage" << "\n"; }

void RPMPage::Update() {
    const auto window_flags = DefaultWindowFlags();
    ImGui::Begin("RPM Data Collection", nullptr, window_flags);
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
    std::vector<double>      time, wheel, engine;

    {
        std::lock_guard lock{m_Context->DataMutex};
        time = m_Context->Data.getTime();

        const auto& rpm_data = m_Context->Data.getRPMData();
        raw_data             = rpm_data.RawLines;
        wheel                = rpm_data.WheelRPM;
        engine               = rpm_data.EngineRPM;
    }

    for (const auto& msg : raw_data) {
        ImGui::TextUnformatted(msg.c_str());
    }

    ImGui::EndChild();

    // Right Side

    ImGui::NextColumn();

    ImGui::BeginChild("Graph Child");

    ImGui::Text("RPM Graph");
    ImGui::Separator();

    if (ImPlot::BeginPlot("RPM Over Time", {-1, -1})) {
        if (!time.empty()) {
            if (!wheel.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), wheel.data(), time.size());
            }

            if (!engine.empty()) {
                ImPlot::PlotLine("Engine Speed", time.data(), engine.data(), time.size());
            }
        }
        ImPlot::EndPlot();
    }

    ImGui::EndChild();

    ImGui::End();
}
