#include <format>

#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"

#include "app/common/plot.hpp"
#include "app/common/scope.hpp"
#include "app/pages/shock.hpp"
#include "app/style.hpp"

void ShockPage::OnEnter() { LOG_INFO("Entered ShockPage"); }
void ShockPage::OnExit() { LOG_INFO("Exited ShockPage"); }

void ShockPage::Update() {
    if (const ImGuiScope<ImGui::EndTable> split{IMSCOPE_FN(ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable))}) {
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
    }
}

void ShockPage::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (const ImGuiScope<ImGui::EndChild> data{IMSCOPE_FN(ImGui::BeginChild("##datalog"))}) {
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        m_TextUtils.DrawStartLoggingButton();
        ImGui::SameLine();
        m_TextUtils.DrawDataDownloadButton(raw_lines, m_DownloadFDText);
        ImGui::SameLine();

        HEADER(TextUtils::DrawInputBox("##extra_shock", m_DownloadFDText, "File descriptor"));
        ImGui::Separator();
        TextUtils::DrawDataLog(raw_lines);
    }
}

void ShockPage::DrawRHS(const std::vector<double>& time,
                        const std::vector<double>& fr,
                        const std::vector<double>& fl,
                        const std::vector<double>& br,
                        const std::vector<double>& bl) {
    if (const ImGuiScope<ImGui::EndChild> graph{IMSCOPE_FN(ImGui::BeginChild("##graph"))}) {
        const auto sync_lt = m_Context->Backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt ? std::format("Shock Travel Data from {}", sync_lt.value().String())
                    : "No Synced Time";

        if (const ImGuiScope<ImPlot::EndPlot, REQUIRE_ALIVE_FOR_DTOR> plot{
                IMSCOPE_FN(ImPlot::BeginPlot(plot_title.c_str(), {-1, -1}))}) {
            PlotUtils::PlotIfNonEmpty("Front Right Shock Travel", time, fr);
            PlotUtils::PlotIfNonEmpty("Front Left Shock Travel", time, fl);
            PlotUtils::PlotIfNonEmpty("Rear Right Shock Travel", time, br);
            PlotUtils::PlotIfNonEmpty("Rear Left Shock Travel", time, bl);
        }
    }
}
