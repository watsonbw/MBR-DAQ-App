#include <cstdint>
#include <iostream>

#include <algorithm>
#include <numeric>

#include <imgui.h>
#include <implot.h>

#include "app/context.hpp"
#include "app/page.hpp"
#include "app/style.hpp"

static ImGuiWindowFlags DefaultWindowFlags() {
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    window_flags |= ImGuiWindowFlags_NoNavFocus;
    return window_flags;
}

void HomePage::OnEnter() { std::cout << "[INFO]: Entered HomePage" << "\n"; }

void HomePage::Update() {
    const auto window_flags = DefaultWindowFlags();
    if (ImGui::Begin("Home Page", nullptr, window_flags)) {}

    ImGui::End();
}

void HomePage::OnExit() { std::cout << "[INFO]: Exitted HomePage" << "\n"; }

void RPMPage::OnEnter() { std::cout << "[INFO]: Enterred RPMPage" << "\n"; }

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

void RPMPage::OnExit() { std::cout << "[INFO]: Exitted RPMPage" << "\n"; }

void ShockPage::OnEnter() { std::cout << "[INFO]: Entered ShockPage" << "\n"; }
void ShockPage::Update() {}
void ShockPage::OnExit() { std::cout << "[INFO]: Exitted ShockPage" << "\n"; }

void ViewPage::OnEnter() { std::cout << "[INFO]: Entered ViewPage" << "\n"; }
void ViewPage::Update() {}
void ViewPage::OnExit() { std::cout << "[INFO]: Exitted ViewPage" << "\n"; }
