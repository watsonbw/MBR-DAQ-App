#include "app/pages/serialmon.hpp"

#include <string>

#include <gsl/util>
#include <imgui.h>
#include <stdx/enum.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"
#include "esp32/serial.hpp"

namespace mbr::pages {

void serial_page::on_enter() { log_info(context_->log, "Entered SerialPage"); }
void serial_page::on_exit() { log_info(context_->log, "Exited SerialPage"); }

void serial_page::update() {
    PROFILE_FUNCTION();
    const f32 full_height   = ImGui::GetContentRegionAvail().y;
    const f32 top_height    = full_height * 0.66F;
    const f32 bottom_height = full_height - top_height;

    if (ImGui::BeginChild("##topsec", {0, top_height})) {
        const auto cleanup_top{gsl::finally(ImGui::EndChild)};
        if (ImGui::BeginTable("##topsplt", 2, ImGuiTableFlags_BordersInnerV)) {
            const auto cleanup_split{gsl::finally(ImGui::EndTable)};
            ImGui::TableNextColumn();
            draw_top_lhs();
            ImGui::TableNextColumn();
            draw_top_rhs();
        }
    }
    ImGui::Separator();

    if (ImGui::BeginChild("##botsec", {0, bottom_height})) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        draw_bottom();
    }
}

void serial_page::draw_top_lhs() {
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        BOLD_HEADER(ImGui::Text("Serial Monitor Settings"));

        ImGui::Separator();
        if (text_drawer_.start_serial_button()) {
            if (!context_->backend->get_serial_manager().is_running()) {
                context_->backend->get_serial_manager().start();
            } else {
                context_->backend->get_serial_manager().stop();
            }
        }
        ImGui::SameLine();
        text_drawer_.send_data_button();
        if (context_->backend->get_serial_manager().should_send_data) {
            context_->backend->get_serial_manager().send_data(context_->backend->get_data().get_current_line());
        }
        ImGui::Separator();

        auto all_ports = context_->backend->get_serial_manager().get_all_ports();
        ImGui::SetNextItemWidth(250.0F);
        if (ImGui::BeginCombo("##port_dropdown", "Select Ports to Send Data")) {
            const auto cleanup{gsl::finally(ImGui::EndCombo)};
            for (const auto& port : all_ports) {
                const bool is_selected = context_->backend->get_serial_manager().is_port_selected(port);
                if (ImGui::Selectable(
                        port.c_str(), is_selected, ImGuiSelectableFlags_NoAutoClosePopups)) {
                    if (!is_selected) {
                        context_->backend->get_serial_manager().add_port(port);
                        log_info(context_->log, "Added port {}", port);
                    } else {
                        context_->backend->get_serial_manager().remove_port(port);
                        log_info(context_->log, "Removed port {}", port);
                    }
                }
            }
        }

        ImGui::SameLine();
        ImGui::TextUnformatted("Chosen Ports: ");
        ImGui::SameLine();
        for (const auto& port : context_->backend->get_serial_manager().get_chosen_ports()) {
            ImGui::TextUnformatted(port.c_str());
            ImGui::SameLine();
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Baud Rate");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(100.0F);
        if (ImGui::BeginCombo(
                "##baud_dropdown",
                baud_rate_string(context_->backend->get_serial_manager().get_baud_rate()))) {
            const auto cleanup_combo{gsl::finally(ImGui::EndCombo)};
            for (const auto rate : stdx::enum_range<baud_rate_t>()) {
                const bool is_selected =
                    (context_->backend->get_serial_manager().get_baud_rate() == rate);
                if (ImGui::Selectable(baud_rate_string(rate), is_selected)) {
                    context_->backend->get_serial_manager().change_baud_rate(rate);
                }
                if (is_selected) { ImGui::SetItemDefaultFocus(); }
            }
        }
    }
}

void serial_page::draw_top_rhs() {
    BOLD_HEADER(ImGui::Text("LOG Info"));

    ImGui::Separator();

    if (ImGui::BeginChild("##errscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto        cleanup{gsl::finally(ImGui::EndChild)};
        const std::string all_errors = context_->logger.get_streamed_logs();
        ImGui::TextUnformatted(all_errors.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
    }
}

void serial_page::draw_bottom() {
    if (utils::draw_input_box("##command",
                              serial_buffer_,
                              "Send Serial Data Here",
                              1900.0F,
                              ImGuiInputTextFlags_EnterReturnsTrue)) {
        context_->backend->get_serial_manager().send_data(serial_buffer_ + "\n");
        serial_buffer_ = {};
        ImGui::SetKeyboardFocusHere(-1);
    }
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        ImGui::Separator();
        utils::draw_data_log(context_->backend->get_serial_manager().return_data_stream());
    }
}

} // namespace mbr::pages
