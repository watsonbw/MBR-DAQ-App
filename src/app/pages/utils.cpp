#include "app/pages/utils.hpp"

#include <cerrno>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

// clang-format off
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <misc/cpp/imgui_stdlib.cpp> // NOLINT
// clang-format on

#include <fmt/format.h>
#include <gsl/span>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "app/context.hpp"
#include "app/style.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

namespace mbr::pages::utils {

void text_drawers::start_logging_button() {
    HEADER({
        if (ImGui::Button(context_->backend->is_logging() ? "Stop Logging" : "Start Logging")) {
            context_->backend->set_logging(!context_->backend->is_logging());
        }
    });
}

bool text_drawers::start_serial_button() {
    bool clicked = false;
    HEADER({
        if (ImGui::Button(context_->backend->get_serial_manager().is_serial_write() ? "Stop Serial"
                                                                            : "Start Serial")) {
            context_->backend->get_serial_manager().set_serial_write(
                !context_->backend->get_serial_manager().is_serial_write());
            clicked = true;
        }
    });
    return clicked;
}

void text_drawers::send_data_button() {
    HEADER({
        if (ImGui::Button(context_->backend->get_serial_manager().should_send_data() ? "Stop Data Send"
                                                                             : "Send Data")) {
            context_->backend->get_serial_manager().set_send_data(
                !context_->backend->get_serial_manager().should_send_data());
        }
    });
}

void text_drawers::data_download_button(const std::vector<std::string>& raw_lines,
                                        std::string&                    buf) {
    HEADER({
        if (ImGui::Button("Download Data")) {
            const date_time dt;
            std::string     filepath;
            if (!buf.empty()) {
                filepath = fmt::format("{}_{}.txt", dt.to_string(date_time::fmt_t::TEXT_FILE), buf);
            } else {
                filepath = fmt::format("{}.txt", dt.to_string(date_time::fmt_t::TEXT_FILE));
            }

            if (raw_lines.empty()) {
                log_warn(context_->log, "Cannot download data as the data buffer is empty!");
            } else {
                std::ofstream out{filepath};
                if (!out.is_open()) {
                    log_error(
                        context_->log, "Failed to open output file: {}", std::strerror(errno));
                } else {
                    for (const auto& line : raw_lines) { out << line << "\n"; }
                    buf = {};
                }
            }
        }
    });
}

bool draw_input_box(const char*               label,
                    std::string&              buf,
                    stdx::option<const char*> hint,
                    f32                       width_scale,
                    ImGuiInputTextFlags       flags) {
    ImGui::SetNextItemWidth(width_scale);
    if (hint) { return ImGui::InputTextWithHint(label, *hint, &buf, flags); }
    return ImGui::InputText(label, &buf, flags);
}

void draw_data_log(gsl::span<const std::string> raw_lines) {
    for (const auto& msg : raw_lines) { ImGui::TextUnformatted(msg.c_str()); }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
}

} // namespace mbr::pages::utils
