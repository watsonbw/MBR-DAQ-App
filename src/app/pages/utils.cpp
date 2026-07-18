#include <fstream>

// clang-format off
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <misc/cpp/imgui_stdlib.cpp> // NOLINT
// clang-format on

#include <fmt/format.h>

#include "app/context.hpp"
#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

namespace mbr::pages::utils {

void text_drawers::start_logging_button() {
    HEADER({
        if (ImGui::Button(m_Context->Backend->IsLogging ? "Stop Logging" : "Start Logging")) {
            m_Context->Backend->IsLogging = !m_Context->Backend->IsLogging;
        }
    });
}

bool text_drawers::start_serial_button() {
    bool clicked = false;
    HEADER({
        if (ImGui::Button(m_Context->Backend->SerialMan.IsSerialWrite ? "Stop Serial"
                                                                      : "Start Serial")) {
            m_Context->Backend->SerialMan.IsSerialWrite =
                !m_Context->Backend->SerialMan.IsSerialWrite;
            clicked = true;
        }
    });
    return clicked;
}

void text_drawers::send_data_button() {
    HEADER({
        if (ImGui::Button(m_Context->Backend->SerialMan.m_SendData ? "Stop Data Send"
                                                                   : "Send Data")) {
            m_Context->Backend->SerialMan.m_SendData = !m_Context->Backend->SerialMan.m_SendData;
        }
    });
}

void text_drawers::data_download_button(const std::vector<std::string>& raw_lines,
                                        std::string&                    buf) {
    HEADER({
        if (ImGui::Button("Download Data")) {
            const DateTime dt;
            std::string    filepath;
            if (!buf.empty()) {
                filepath =
                    fmt::format("{}_{}.txt", dt.String(DateTime::StringFormat::TEXT_FILE), buf);
            } else {
                filepath = fmt::format("{}.txt", dt.String(DateTime::StringFormat::TEXT_FILE));
            }

            if (raw_lines.empty()) {
                LOG_WARN("Cannot download data as the data buffer is empty!");
            } else {
                std::ofstream out{filepath};
                if (!out.is_open()) {
                    LOG_ERROR("Failed to open output file: ", std::strerror(errno));
                } else {
                    for (const auto& line : raw_lines) { out << line << "\n"; }
                    buf = {};
                }
            }
        }
    });
}

bool draw_input_box(const char*                label,
                    std::string&               buf,
                    std::optional<const char*> hint,
                    float                      width_scale,
                    ImGuiInputTextFlags        flags) {
    ImGui::SetNextItemWidth(width_scale);
    if (hint) { return ImGui::InputTextWithHint(label, *hint, &buf, flags); }
    return ImGui::InputText(label, &buf, flags);
}

void draw_data_log(gsl::span<const std::string> raw_lines) {
    for (const auto& msg : raw_lines) { ImGui::TextUnformatted(msg.c_str()); }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
}

} // namespace mbr::pages::utils
