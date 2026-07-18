#include <fmt/format.h>
#include <gsl/span>
#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"

#include "app/pages/shock.hpp"
#include "app/pages/utils.hpp"
#include "app/style.hpp"

namespace mbr {

void ShockPage::OnEnter() { LOG_INFO("Entered ShockPage"); }
void ShockPage::OnExit() { LOG_INFO("Exited ShockPage"); }

void ShockPage::Update() {
    if (ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto cleanup_split{gsl::finally(ImGui::EndTable)};
        const auto data = context_->backend->PackData();

        ImGui::TableNextColumn();
        DrawLHS(data.RawLines);
        ImGui::TableNextColumn();
        DrawRHS(data.TimeMinutesNormalized,
                data.Series.at("FR"),
                data.Series.at("FL"),
                data.Series.at("RR"),
                data.Series.at("RL"));
    }
}

void ShockPage::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup_data{gsl::finally(ImGui::EndChild)};
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        m_TextUtils.start_logging_button();
        ImGui::SameLine();
        m_TextUtils.data_download_button(raw_lines, m_DownloadFDText);
        ImGui::SameLine();

        HEADER(pages::utils::draw_input_box("##extra_shock", m_DownloadFDText, "File descriptor"));
        ImGui::Separator();
        pages::utils::draw_data_log(raw_lines);
    }
}

void ShockPage::DrawRHS(gsl::span<const double> time,
                        gsl::span<const double> fr,
                        gsl::span<const double> fl,
                        gsl::span<const double> br,
                        gsl::span<const double> bl) {
    if (ImGui::BeginChild("##graph")) {
        const auto cleanup_graph{gsl::finally(ImGui::EndChild)};
        const auto sync_lt = context_->backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt ? fmt::format("Shock Travel Data from {}", sync_lt.value().to_string())
                    : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            const auto cleanup_plot{gsl::finally(ImPlot::EndPlot)};
            pages::utils::plot_if_non_empty<double>("Front Right Shock Travel", time, fr);
            pages::utils::plot_if_non_empty<double>("Front Left Shock Travel", time, fl);
            pages::utils::plot_if_non_empty<double>("Rear Right Shock Travel", time, br);
            pages::utils::plot_if_non_empty<double>("Rear Left Shock Travel", time, bl);
        }
    }
}

} // namespace mbr
