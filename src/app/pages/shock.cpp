#include <format>
#include <fstream>

#include <imgui.h>
#include <implot.h>

#include "core/log.hpp"

#include "app/pages/shock.hpp"

void ShockPage::OnEnter() { LOG_INFO("Entered ShockPage"); }
void ShockPage::OnExit() { LOG_INFO("Exited ShockPage"); }

void ShockPage::Update() {
    const auto window_flags = DefaultWindowFlags();

    std::vector<std::string> raw_data;
    std::vector<double>      time, fr, fl, br, bl;

    {
        std::lock_guard lock{m_Context->Backend->DataMutex};
        time = m_Context->Backend->Data.GetTime();

        raw_data               = m_Context->Backend->Data.GetRawLines();
        const auto& shock_data = m_Context->Backend->Data.GetShockData();
        fr                     = shock_data.FrontRight;
        fl                     = shock_data.FrontLeft;
        br                     = shock_data.BackRight;
        bl                     = shock_data.BackRight;
    }

    ImGui::Begin("Shock Data Collection", nullptr, window_flags);

    if (ImGui::BeginTable(
            "ViewSplit", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable)) {
        ImGui::TableNextColumn();
        DrawLHS(raw_data);
        ImGui::TableNextColumn();
        DrawRHS(time, fr, fl, br, bl);
        ImGui::EndTable();
    }

    ImGui::End();
}

void ShockPage::DrawLHS(std::vector<std::string> raw_data) {
    static char extraTextBuffer[256] = "";
    DateTime    dt;

    ImGui::BeginChild("Data Log Child");
    ImGui::Text("Data Log");
    ImGui::Separator();
    {
        ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);

        if (ImGui::Button(m_Context->Backend->IsLogging ? "Stop Logging" : "Start Logging")) {
            m_Context->Backend->IsLogging = !m_Context->Backend->IsLogging;
        }

        ImGui::PopFont();
    }

    ImGui::SameLine();

    {
        ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);
        if (ImGui::Button("Download Data")) {
            std::string              extra = extraTextBuffer;
            std::string              final;
            std::vector<std::string> rawlines;

            if (!extra.empty()) {
                final = dt.String(DateTime::StringFormat::TEXT_FILE) + "_ " + extra + ".txt";
            } else {
                final = dt.String(DateTime::StringFormat::TEXT_FILE) + ".txt";
            }

            rawlines = m_Context->Backend->Data.GetRawLines();
            std::ofstream out(final);
            if (!out.is_open()) { LOG_ERROR("OFSTREAM DIDN'T OPEN", std::strerror(errno)); }

            for (const auto& line : rawlines)
                out << line << "\n";

            extraTextBuffer[0] = '\0';
        }
        ImGui::PopFont();
    }

    ImGui::SameLine();

    {
        ImGui::PushFont(m_Context->Style.DefaultFonts.Regular, 36.0f);

        ImGui::SetNextItemWidth(200.0f);

        ImGui::InputText("##extra", extraTextBuffer, IM_ARRAYSIZE(extraTextBuffer));

        ImGui::PopFont();
    }

    ImGui::Separator();

    for (const auto& msg : raw_data) {
        ImGui::TextUnformatted(msg.c_str());
    }

    ImGui::EndChild();
}

void ShockPage::DrawRHS(std::vector<double> time,
                        std::vector<double> fr,
                        std::vector<double> fl,
                        std::vector<double> br,
                        std::vector<double> bl) {
    ImGui::NextColumn();

    ImGui::BeginChild("Graph Child");

    const auto plot_title =
        m_Context->Backend->Data.IsSynced
            ? std::format("Shock Travel from {}", m_Context->Backend->Data.SyncLT.String())
            : "Shock Travel from No Synced Time";

    if (ImPlot::BeginPlot(plot_title.c_str(), {-1, -1})) {
        if (!time.empty()) {
            if (!fr.empty()) {
                ImPlot::PlotLine("Front Right Shock Travel",
                                 time.data(),
                                 fr.data(),
                                 std::min(time.size(), fr.size()));
            }
            if (!fl.empty()) {
                ImPlot::PlotLine("Front Left Shock Travel",
                                 time.data(),
                                 fl.data(),
                                 std::min(time.size(), fl.size()));
            }
            if (!br.empty()) {
                ImPlot::PlotLine("Rear Right Shock Travel",
                                 time.data(),
                                 br.data(),
                                 std::min(time.size(), br.size()));
            }
            if (!bl.empty()) {
                ImPlot::PlotLine("Rear Left Shock Travel",
                                 time.data(),
                                 bl.data(),
                                 std::min(time.size(), bl.size()));
            }
        }
        ImPlot::EndPlot();
    }

    ImGui::EndChild();
}
