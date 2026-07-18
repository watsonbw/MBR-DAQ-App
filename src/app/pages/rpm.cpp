#include <fmt/format.h>
#include <gsl/span>
#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/pages/rpm.hpp"
#include "app/pages/utils.hpp"
#include "app/style.hpp"

namespace mbr {

void RPMPage::OnEnter() { LOG_INFO("Entered RPMPage"); }
void RPMPage::OnExit() { LOG_INFO("Exited RPMPage"); }

void RPMPage::Update() {
    if (ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto cleanup_split{gsl::finally(ImGui::EndTable)};
        const auto data = context_->backend->PackData();

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
        m_TextUtils.start_logging_button();
        ImGui::SameLine();
        m_TextUtils.data_download_button(raw_lines, m_DownloadFDText);
        ImGui::SameLine();

        HEADER(pages::utils::draw_input_box("##extra_rpm", m_DownloadFDText, "File descriptor"));
        ImGui::Separator();
        pages::utils::draw_data_log(raw_lines);
    }
}

void RPMPage::DrawRHS(gsl::span<const double> time,
                      gsl::span<const double> wheel,
                      gsl::span<const double> engine) {
    if (ImGui::BeginChild("##graph")) {
        const auto cleanup_graph{gsl::finally(ImGui::EndChild)};
        const auto sync_lt = context_->backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt ? fmt::format("RPM Data from {}", sync_lt.value().String()) : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            const auto cleanup_plot{gsl::finally(ImPlot::EndPlot)};
            pages::utils::plot_if_non_empty<double>("Wheel Speed", time, wheel);
            pages::utils::plot_if_non_empty<double>("Engine Speed", time, engine);
        }
    }
}

} // namespace mbr
