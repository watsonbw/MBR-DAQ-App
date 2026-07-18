#include <gsl/util>
#include <imgui.h>

#include "app/pages/serialmon.hpp"
#include "app/style.hpp"
#include "core/log.hpp"

namespace mbr {

void SerialPage::OnEnter() { LOG_INFO("Entered SerialPage"); }
void SerialPage::OnExit() { LOG_INFO("Exited SerialPage"); }

void SerialPage::Update() {
    const float full_height   = ImGui::GetContentRegionAvail().y;
    const float top_height    = full_height * 0.66F;
    const float bottom_height = full_height - top_height;

    if (ImGui::BeginChild("##topsec", {0, top_height})) {
        const auto cleanup_top{gsl::finally(ImGui::EndChild)};
        if (ImGui::BeginTable("##topsplt", 2, ImGuiTableFlags_BordersInnerV)) {
            const auto cleanup_split{gsl::finally(ImGui::EndTable)};
            ImGui::TableNextColumn();
            DrawTopLHS();
            ImGui::TableNextColumn();
            DrawTopRHS();
        }
    }
    ImGui::Separator();

    if (ImGui::BeginChild("##botsec", {0, bottom_height})) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        DrawBottom();
    }
}

void SerialPage::DrawTopLHS() {
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        BOLD_HEADER(ImGui::Text("Serial Monitor Settings"));

        ImGui::Separator();
        if (m_TextUtils.DrawStartSerialButton()) {
            if (!m_Context->Backend->SerialMan.IsRunning()) {
                m_Context->Backend->SerialMan.Start();
            } else {
                m_Context->Backend->SerialMan.Stop();
            }
        }
        ImGui::SameLine();
        m_TextUtils.DrawSendDataButton();
        if (m_Context->Backend->SerialMan.m_SendData) {
            m_Context->Backend->SerialMan.SendData(m_Context->Backend->Data.GetCurrentLine());
        }
        ImGui::Separator();

        auto all_ports = m_Context->Backend->SerialMan.ExportPorts();
        ImGui::SetNextItemWidth(250.0F);
        if (ImGui::BeginCombo("##port_dropdown", "Select Ports to Send Data")) {
            const auto cleanup{gsl::finally(ImGui::EndCombo)};
            for (auto& [port, ser] : all_ports) {
                const bool is_selected = m_Context->Backend->SerialMan.IsPortSelected(port);
                if (ImGui::Selectable(
                        port.c_str(), is_selected, ImGuiSelectableFlags_NoAutoClosePopups)) {
                    if (!is_selected) {
                        m_Context->Backend->SerialMan.AddPort(port);
                        LOG_INFO("Added port " + port);
                    } else {
                        m_Context->Backend->SerialMan.RemovePort(port);
                        LOG_INFO("Removed port " + port);
                    }
                }
            }
        }

        ImGui::SameLine();
        ImGui::TextUnformatted("Chosen Ports: ");
        ImGui::SameLine();
        for (const auto& port : m_Context->Backend->SerialMan.ReturnChosen()) {
            ImGui::TextUnformatted(port.c_str());
            ImGui::SameLine();
        }
        ImGui::Separator();
        ImGui::TextUnformatted("Baud Rate");
        ImGui::SameLine();

        ImGui::SetNextItemWidth(100.0F);
        if (ImGui::BeginCombo("##baud_dropdown",
                              std::to_string(m_Context->Backend->SerialMan.GetBaudRate())
                                  .c_str())) { // TODO(tcs): This is ugly
            const auto cleanup_combo{gsl::finally(ImGui::EndCombo)};
            // This can stay inside DrawTopLHS or be a static member
            static const uint32_t BAUD_RATES[] = {
                300, 1'200, 2'400, 4'800, 9'600, 19'200, 38'400, 57'600, 115'200};

            for (const uint32_t rate : BAUD_RATES) {
                const bool is_selected = (m_Context->Backend->SerialMan.GetBaudRate() == rate);
                if (ImGui::Selectable(std::to_string(rate).c_str(), is_selected)) {
                    m_Context->Backend->SerialMan.ChangeBaudRate(rate);
                }
                if (is_selected) { ImGui::SetItemDefaultFocus(); }
            }
        }
    }
}

void SerialPage::DrawTopRHS() {
    BOLD_HEADER(ImGui::Text("LOG Info"));

    ImGui::Separator();

    if (ImGui::BeginChild("##errscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar)) {
        const auto        cleanup{gsl::finally(ImGui::EndChild)};
        const std::string all_errors = Log::GetStreamedLogs();
        ImGui::TextUnformatted(all_errors.c_str());
        if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.0F); }
    }
}

void SerialPage::DrawBottom() {
    if (TextUtils::DrawInputBox("##command",
                                m_SerialBuffer,
                                "Send Serial Data Here",
                                1900.0F,
                                ImGuiInputTextFlags_EnterReturnsTrue)) {
        m_Context->Backend->SerialMan.SendData(m_SerialBuffer + "\n");
        m_SerialBuffer = {};
        ImGui::SetKeyboardFocusHere(-1);
    }
    if (ImGui::BeginChild("##datalog")) {
        const auto cleanup{gsl::finally(ImGui::EndChild)};
        ImGui::Separator();
        TextUtils::DrawDataLog(m_Context->Backend->SerialMan.ReturnDataStream());
    }
}

} // namespace mbr
