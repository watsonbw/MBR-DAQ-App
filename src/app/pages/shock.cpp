#include "app/pages/shock.hpp"

#include <string>
#include <vector>

#include <fmt/format.h>
#include <gsl/span>
#include <gsl/util>
#include <imgui.h>
#include <implot.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"

namespace mbr::pages {

void shock_page::on_enter() { log_info(context_->log, "Entered ShockPage"); }
void shock_page::on_exit() { log_info(context_->log, "Exited ShockPage"); }

void shock_page::update() {
    PROFILE_FUNCTION();
    if (ImGui::BeginTable(
            "##viewsplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        const auto cleanup_split{gsl::finally(ImGui::EndTable)};
        const auto data = context_->backend->pack_data();

        ImGui::TableNextColumn();
        draw_lhs(data.raw_lines);
        ImGui::TableNextColumn();
        draw_rhs(data.time_minutes_normalized,
                 data.series.at("FR"),
                 data.series.at("FL"),
                 data.series.at("RR"),
                 data.series.at("RL"));
    }
}

void shock_page::draw_lhs(const std::vector<std::string>& raw_lines) {
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup_data{gsl::finally(ImGui::EndChild)};
        BOLD_HEADER(ImGui::Text("Data Log"));

        ImGui::Separator();
        text_utils_.start_logging_button();
        ImGui::SameLine();
        text_utils_.data_download_button(raw_lines, download_fd_text_);
        ImGui::SameLine();

        HEADER(utils::draw_input_box("##extra_shock", download_fd_text_, "File descriptor"));
        ImGui::Separator();
        utils::draw_data_log(raw_lines);
    }
}

void shock_page::draw_rhs(gsl::span<const f64> time,
                          gsl::span<const f64> fr,
                          gsl::span<const f64> fl,
                          gsl::span<const f64> br,
                          gsl::span<const f64> bl) {
    if (ImGui::BeginChild("##graph")) {
        const auto cleanup_graph{gsl::finally(ImGui::EndChild)};
        const auto sync_lt = context_->backend->get_data().get_sync_lt();
        const auto plot_title =
            sync_lt ? fmt::format("Shock Travel Data from {}", sync_lt.value().to_string())
                    : "No Synced Time";

        if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
            const auto cleanup_plot{gsl::finally(ImPlot::EndPlot)};
            utils::plot_if_non_empty<f64>("Front Right Shock Travel", time, fr);
            utils::plot_if_non_empty<f64>("Front Left Shock Travel", time, fl);
            utils::plot_if_non_empty<f64>("Rear Right Shock Travel", time, br);
            utils::plot_if_non_empty<f64>("Rear Left Shock Travel", time, bl);
        }
    }
}

} // namespace mbr::pages
