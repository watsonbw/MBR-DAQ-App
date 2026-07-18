#include <fmt/format.h>
#include <gsl/span>
#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/pages/rpm.hpp"
#include "app/pages/utils.hpp"
#include "app/style.hpp"

namespace mbr::pages {

void rpm_page::on_enter() { log_info(context_->log, "Entered RPMPage"); }
void rpm_page::on_exit() { log_info(context_->log, "Exited RPMPage"); }

void rpm_page::update() {
    if (ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto cleanup_split{gsl::finally(ImGui::EndTable)};
        const auto data = context_->backend->pack_data();

        ImGui::TableNextColumn();
        DrawLHS(data.raw_lines);
        ImGui::TableNextColumn();
        DrawRHS(data.time_minutes_normalized, data.series.at("W"), data.series.at("E"));
    }
}

void rpm_page::DrawLHS(const std::vector<std::string>& raw_lines) {
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup_data{gsl::finally(ImGui::EndChild)};
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        text_drawer_.start_logging_button();
        ImGui::SameLine();
        text_drawer_.data_download_button(raw_lines, download_fd_text_);
        ImGui::SameLine();

        HEADER(pages::utils::draw_input_box("##extra_rpm", download_fd_text_, "File descriptor"));
        ImGui::Separator();
        pages::utils::draw_data_log(raw_lines);
    }
}

void rpm_page::DrawRHS(gsl::span<const double> time,
                       gsl::span<const double> wheel,
                       gsl::span<const double> engine) {
    if (ImGui::BeginChild("##graph")) {
        const auto cleanup_graph{gsl::finally(ImGui::EndChild)};
        const auto sync_lt    = context_->backend->data.get_sync_lt();
        const auto plot_title = sync_lt
                                    ? fmt::format("RPM Data from {}", sync_lt.value().to_string())
                                    : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            const auto cleanup_plot{gsl::finally(ImPlot::EndPlot)};
            pages::utils::plot_if_non_empty<double>("Wheel Speed", time, wheel);
            pages::utils::plot_if_non_empty<double>("Engine Speed", time, engine);
        }
    }
}

} // namespace mbr::pages
