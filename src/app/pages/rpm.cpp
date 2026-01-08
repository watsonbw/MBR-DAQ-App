#include <format>

#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/common/plot.hpp"
#include "app/common/scope.hpp"
#include "app/pages/rpm.hpp"
#include "app/style.hpp"

void RPMPage::OnEnter() { LOG_INFO("Entered RPMPage"); }
void RPMPage::OnExit() { LOG_INFO("Exited RPMPage"); }

void RPMPage::Update() {
    if (const ImGuiScope<ImGui::EndTable> split{IMSCOPE_FN(ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable))}) {
        const auto  data = m_Context->Backend->PackData();
        const auto& rpm  = data.RPM;

        ImGui::TableNextColumn();
        DrawLHS(data.RawLines);
        ImGui::TableNextColumn();
        DrawRHS(data.TimeMinutesNormalized, rpm.WheelRPM, rpm.EngineRPM);
    }
}

void RPMPage::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (const ImGuiScope<ImGui::EndChild> data{IMSCOPE_FN(ImGui::BeginChild("##datalog"))}) {
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        m_TextUtils.DrawStartLoggingButton();
        ImGui::SameLine();
        m_TextUtils.DrawDataDownloadButton(raw_lines, m_DownloadFDText);
        ImGui::SameLine();

        HEADER(TextUtils::DrawInputBox("##extra_rpm", m_DownloadFDText, "File descriptor"));
        ImGui::Separator();
        TextUtils::DrawDataLog(raw_lines);
    }
}

void RPMPage::DrawRHS(const std::vector<double>& time,
                      const std::vector<double>& wheel,
                      const std::vector<double>& engine) {
    if (const ImGuiScope<ImGui::EndChild> graph{IMSCOPE_FN(ImGui::BeginChild("##graph"))}) {
        const auto sync_lt = m_Context->Backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt ? std::format("RPM Data from {}", sync_lt.value().String()) : "No Synced Time";

        if (const ImGuiScope<ImPlot::EndPlot, REQUIRE_ALIVE_FOR_DTOR> plot{
                IMSCOPE_FN(ImPlot::BeginPlot(plot_title.c_str(), {-1, -1}))}) {
            PlotUtils::PlotIfNonEmpty("Wheel Speed", time, wheel);
            PlotUtils::PlotIfNonEmpty("Engine Speed", time, engine);
        }
    }
}
