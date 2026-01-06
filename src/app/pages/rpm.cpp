#include <format>

#include <imgui.h>
#include <implot.h>

#include "app/style.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

#include "app/pages/common/plot.hpp"
#include "app/pages/rpm.hpp"

void RPMPage::OnEnter() { LOG_INFO("Entered RPMPage"); }
void RPMPage::OnExit() { LOG_INFO("Exited RPMPage"); }

void RPMPage::Update() {
    const auto window_flags = DefaultWindowFlags();
    if (ImGui::Begin("RPM Data Collection", nullptr, window_flags)) {
        if (ImGui::BeginTable(
                "ViewSplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
            const auto  data = m_Context->Backend->PackData();
            const auto& rpm  = data.RPM;

            ImGui::TableNextColumn();
            DrawLHS(data.RawLines);
            ImGui::TableNextColumn();
            DrawRHS(data.TimeMinutesNormalized, rpm.WheelRPM, rpm.EngineRPM);
            ImGui::EndTable();
        }
    }
    ImGui::End();
}

void RPMPage::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (ImGui::BeginChild("Data Log Child")) {
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        m_TextUtils.DrawStartLoggingButton();
        ImGui::SameLine();
        m_TextUtils.DrawDataDownloadButton(raw_lines, m_DownloadFDText);
        ImGui::SameLine();

        HEADER(TextUtils::DrawInputBox("##extra_rpm", m_DownloadFDText, "File descriptor"));
        ImGui::Separator();
        m_TextUtils.DrawDataLog(raw_lines);
    }
    ImGui::EndChild();
}

void RPMPage::DrawRHS(const std::vector<double>& time,
                      const std::vector<double>& wheel,
                      const std::vector<double>& engine) {
    if (ImGui::BeginChild("Graph Child", {0, 0})) {
        const auto sync_lt    = m_Context->Backend->Data.GetSyncLT();
        const auto plot_title = sync_lt.has_value()
                                    ? std::format("RPM Data from {}", sync_lt.value().String())
                                    : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            PlotUtils::PlotIfNonEmpty("Wheel Speed", time, wheel);
            PlotUtils::PlotIfNonEmpty("Engine Speed", time, engine);
            ImPlot::EndPlot();
        }
    }
    ImGui::EndChild();
}
