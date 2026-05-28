#include <format>

#include <imgui.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/common/plot.hpp"
#include "app/common/scope.hpp"
#include "app/pages/serialmon.hpp"
#include "app/style.hpp"

void SerialPage::OnEnter() { LOG_INFO("Entered SerialPage"); }
void SerialPage::OnExit() { LOG_INFO("Exited SerialPage"); }

void SerialPage::Update() {
    const float full_height   = ImGui::GetContentRegionAvail().y;
    const float top_height    = full_height * 0.66F;
    const float bottom_height = full_height - top_height;

    if (const ImGuiScope<ImGui::EndChild> top_section{
            IMSCOPE_FN(ImGui::BeginChild("##topsec", {0, top_height}))}) {
        if (const ImGuiScope<ImGui::EndTable> split{
                IMSCOPE_FN(ImGui::BeginTable("##topsplt", 2, ImGuiTableFlags_BordersInnerV))}) {
            ImGui::TableNextColumn();
            DrawTopLHS();
            ImGui::TableNextColumn();
            DrawTopRHS();
        }
    }
    ImGui::Separator();

    if (const ImGuiScope<ImGui::EndChild> bottom_section{
            IMSCOPE_FN(ImGui::BeginChild("##botsec", {0, bottom_height}))}) {
        DrawBottom();
    }
}

void SerialPage::DrawTopLHS() {
    if (const ImGuiScope<ImGui::EndChild> data{IMSCOPE_FN(ImGui::BeginChild("##datalog"))}) {
        BOLD_HEADER(ImGui::Text("Serial Monitor Settings"));

        ImGui::Separator();
        if (m_TextUtils.DrawStartSerialButton()){
            if(!m_Context->Backend->SerialMan.IsRunning()){
                m_Context->Backend->SerialMan.Start();
            } else {
                m_Context->Backend->SerialMan.Stop();
            }
        }
        ImGui::SameLine();
        m_TextUtils.DrawSendDataButton();
        if(m_Context->Backend->SerialMan.m_SendData){
            m_Context->Backend->SerialMan.SendData(m_Context->Backend->Data.GetCurrentLine());
        }
        ImGui::Separator();

        const std::string dropdown  = "Select Ports to Send Data";
        auto              all_ports = m_Context->Backend->SerialMan.ExportPorts();
        ImGui::SetNextItemWidth(250.0F);
        if (const ImGuiScope<ImGui::EndCombo, REQUIRE_ALIVE_FOR_DTOR> port_select{
                IMSCOPE_FN(ImGui::BeginCombo("##port_dropdown", dropdown.c_str()))}) {
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
        if (const ImGuiScope<ImGui::EndCombo, REQUIRE_ALIVE_FOR_DTOR> baud_combo{
                IMSCOPE_FN(ImGui::BeginCombo(
                    "##baud_dropdown",
                    std::to_string(m_Context->Backend->SerialMan.GetBaudRate()).c_str()))}) {
            // This can stay inside DrawTopLHS or be a static member
            static const uint32_t BAUD_RATES[] = {
                300, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200};

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

    if (const ImGuiScope<ImGui::EndChild> scroll{IMSCOPE_FN(ImGui::BeginChild(
            "##errscroll", {0, 0}, false, ImGuiWindowFlags_HorizontalScrollbar))}) {
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
    if (const ImGuiScope<ImGui::EndChild> data{IMSCOPE_FN(ImGui::BeginChild("##datalog"))}) {
        ImGui::Separator();
        TextUtils::DrawDataLog(m_Context->Backend->SerialMan.ReturnDataStream());
    }
}
