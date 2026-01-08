#include <format>
#include <fstream>

// clang-format off
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#include <misc/cpp/imgui_stdlib.cpp> // NOLINT
// clang-format on

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/common/text.hpp"
#include "app/context.hpp"
#include "app/style.hpp"

void TextUtils::DrawStartLoggingButton() {
    HEADER({
        if (ImGui::Button(m_Context->Backend->IsLogging ? "Stop Logging" : "Start Logging")) {
            m_Context->Backend->IsLogging = !m_Context->Backend->IsLogging;
        }
    });
}

void TextUtils::DrawDataDownloadButton(const std::vector<std::string>& raw_lines,
                                       std::string&                    buf) {
    HEADER({
        if (ImGui::Button("Download Data")) {
            const DateTime dt;
            std::string    filepath;
            if (!buf.empty()) {
                filepath =
                    std::format("{}_{}.txt", dt.String(DateTime::StringFormat::TEXT_FILE), buf);
            } else {
                filepath = std::format("{}.txt", dt.String(DateTime::StringFormat::TEXT_FILE));
            }

            if (raw_lines.empty()) {
                LOG_WARN("Cannot download data as the data buffer is empty!");
            } else {
                std::ofstream out{filepath};
                if (!out.is_open()) {
                    LOG_ERROR("Failed to open output file: ", std::strerror(errno));
                } else {
                    for (const auto& line : raw_lines) {
                        out << line << "\n";
                    }
                    buf = {};
                }
            }
        }
    });
}

bool TextUtils::DrawInputBox(const char*                label,
                             std::string&               buf,
                             std::optional<const char*> hint,
                             float                      width_scale,
                             ImGuiInputTextFlags        flags) {
    ImGui::SetNextItemWidth(width_scale);
    if (hint) { return ImGui::InputTextWithHint(label, hint.value(), &buf, flags); }
    return ImGui::InputText(label, &buf, flags);
}

void TextUtils::DrawDataLog(const std::vector<std::string>& raw_lines) {
    for (const auto& msg : raw_lines) {
        ImGui::TextUnformatted(msg.c_str());
    }
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
}
