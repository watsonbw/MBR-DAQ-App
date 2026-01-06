#include <format>

#include <imgui.h>
#include <implot.h>

#include "app/style.hpp"
#include "core/log.hpp"

#include "app/pages/common/plot.hpp"
#include "app/pages/shock.hpp"

void ShockPage::OnEnter() { LOG_INFO("Entered ShockPage"); }
void ShockPage::OnExit() { LOG_INFO("Exited ShockPage"); }

void ShockPage::Update() {
    if (ImGui::BeginTable(
            "ViewSplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto  data  = m_Context->Backend->PackData();
        const auto& shock = data.Shock;

        ImGui::TableNextColumn();
        DrawLHS(data.RawLines);
        ImGui::TableNextColumn();
        DrawRHS(data.TimeMinutesNormalized,
                shock.FrontRight,
                shock.FrontLeft,
                shock.BackRight,
                shock.BackLeft);
        ImGui::EndTable();
    }
}

void ShockPage::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (ImGui::BeginChild("Data Log Child")) {
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        m_TextUtils.DrawStartLoggingButton();
        ImGui::SameLine();
        m_TextUtils.DrawDataDownloadButton(raw_lines, m_DownloadFDText);
        ImGui::SameLine();

        HEADER(TextUtils::DrawInputBox("##extra_shock", m_DownloadFDText, "File descriptor"));
        ImGui::Separator();
        m_TextUtils.DrawDataLog(raw_lines);
    }
    ImGui::EndChild();
}

void ShockPage::DrawRHS(const std::vector<double>& time,
                        const std::vector<double>& fr,
                        const std::vector<double>& fl,
                        const std::vector<double>& br,
                        const std::vector<double>& bl) {
    ImGui::NextColumn();

    if (ImGui::BeginChild("Graph Child")) {
        const auto sync_lt = m_Context->Backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt.has_value() ? std::format("Shock Travel Data from {}", sync_lt.value().String())
                                : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            PlotUtils::PlotIfNonEmpty("Front Right Shock Travel", time, fr);
            PlotUtils::PlotIfNonEmpty("Front Left Shock Travel", time, fl);
            PlotUtils::PlotIfNonEmpty("Rear Right Shock Travel", time, br);
            PlotUtils::PlotIfNonEmpty("Rear Left Shock Travel", time, bl);
            ImPlot::EndPlot();
        }
    }
    ImGui::EndChild();
}
