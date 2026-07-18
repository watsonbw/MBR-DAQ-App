#include <fmt/format.h>
#include <gsl/span>
#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/common/plot_utils.hpp"
#include "app/pages/rpm.hpp"
#include "app/style.hpp"

namespace mbr {

void RPMPage::OnEnter() { LOG_INFO("Entered RPMPage"); }
void RPMPage::OnExit() { LOG_INFO("Exited RPMPage"); }

void RPMPage::Update() {
    if (ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto cleanup_split{gsl::finally(ImGui::EndTable)};
        const auto data = m_Context->Backend->PackData();

        ImGui::TableNextColumn();
        DrawLHS(data.RawLines);
        ImGui::TableNextColumn();
        DrawRHS(data.TimeMinutesNormalized, data.Series.at("W"), data.Series.at("E"));
    }
}

void RPMPage::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup_data{gsl::finally(ImGui::EndChild)};
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

void RPMPage::DrawRHS(gsl::span<const double> time,
                      gsl::span<const double> wheel,
                      gsl::span<const double> engine) {
    if (ImGui::BeginChild("##graph")) {
        const auto cleanup_graph{gsl::finally(ImGui::EndChild)};
        const auto sync_lt = m_Context->Backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt ? fmt::format("RPM Data from {}", sync_lt.value().String()) : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            const auto cleanup_plot{gsl::finally(ImPlot::EndPlot)};
            plot_utils::plot_if_non_empty<double>("Wheel Speed", time, wheel);
            plot_utils::plot_if_non_empty<double>("Engine Speed", time, engine);
        }
    }
}

} // namespace mbr
