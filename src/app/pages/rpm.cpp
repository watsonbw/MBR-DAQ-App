#include <format>
#include <fstream>

#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/pages/rpm.hpp"

void RPMPage::OnEnter() { LOG_INFO("Entered RPMPage"); }
void RPMPage::OnExit() { LOG_INFO("Exited RPMPage"); }

void RPMPage::Update() {
    const auto               window_flags = DefaultWindowFlags();
    std::vector<std::string> raw_data;
    std::vector<double>      time, wheel, engine;

    {
        std::lock_guard lock{m_Context->Backend->DataMutex};
        time = m_Context->Backend->Data.GetTime();

        const auto& rpm_data = m_Context->Backend->Data.GetRPMData();
        raw_data             = m_Context->Backend->Data.GetRawLines();
        wheel                = rpm_data.WheelRPM;
        engine               = rpm_data.EngineRPM;
    }
    ImGui::Begin("RPM Data Collection", nullptr, window_flags);

    if (ImGui::BeginTable(
            "ViewSplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        ImGui::TableNextColumn();
        DrawLHS(raw_data);
        ImGui::TableNextColumn();
        DrawRHS(time, wheel, engine);
        ImGui::EndTable();
    }

    ImGui::End();
}

void RPMPage::DrawLHS(std::vector<std::string> raw_data) {
    DateTime    dt;
    static char extraTextBuffer[256] = "";
    ImGui::BeginChild("Data Log Child");
    ImGui::Text("Data Log");
    ImGui::Separator();
    {
        ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);

        if (ImGui::Button(m_Context->Backend->IsLogging ? "Stop Logging" : "Start Logging")) {
            m_Context->Backend->IsLogging = !m_Context->Backend->IsLogging;
        }

        ImGui::PopFont();
    }

    ImGui::SameLine();

    {
        ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);
        if (ImGui::Button("Download Data")) {
            std::string              extra = extraTextBuffer;
            std::string              final;
            std::vector<std::string> rawlines;

            if (!extra.empty()) {
                final = dt.String(DateTime::StringFormat::TEXT_FILE) + "_" + extra + ".txt";
            } else {
                final = dt.String(DateTime::StringFormat::TEXT_FILE) + ".txt";
            }

            rawlines = m_Context->Backend->Data.GetRawLines();
            std::ofstream out(final);
            if (!out.is_open()) { LOG_ERROR("OFSTREAM DIDN'T OPEN", std::strerror(errno)); }

            for (const auto& line : rawlines)
                out << line << "\n";

            extraTextBuffer[0] = '\0';
        }
        ImGui::PopFont();
    }

    ImGui::SameLine();

    {
        ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);

        ImGui::SetNextItemWidth(200.0f);

        ImGui::InputText("##extra", extraTextBuffer, IM_ARRAYSIZE(extraTextBuffer));

        ImGui::PopFont();
    }

    ImGui::Separator();

    for (const auto& msg : raw_data) {
        ImGui::TextUnformatted(msg.c_str());
    }

    ImGui::EndChild();
}

void RPMPage::DrawRHS(std::vector<double> time,
                      std::vector<double> wheel,
                      std::vector<double> engine) {
    ImGui::BeginChild("Graph Child", {0, 0});

    const auto plot_title =
        m_Context->Backend->Data.IsSyncedToZero
            ? std::format("RPM from {}", m_Context->Backend->Data.SyncLT.String())
            : "RPM from No Synced Time";

    if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
        if (!time.empty()) {
            if (!wheel.empty()) {
                ImPlot::PlotLine(
                    "Wheel Speed", time.data(), wheel.data(), std::min(time.size(), wheel.size()));
            }

            if (!engine.empty()) {
                ImPlot::PlotLine("Engine Speed",
                                 time.data(),
                                 engine.data(),
                                 std::min(time.size(), engine.size()));
            }
        }
        ImPlot::EndPlot();
    }

    ImGui::EndChild();
}
