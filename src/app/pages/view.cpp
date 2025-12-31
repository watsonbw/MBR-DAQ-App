#include <iostream>

#include <tinyfiledialogs.h>

#include <imgui.h>

#include "app/pages/view.hpp"

void ViewPage::OnEnter() { std::cout << "[INFO]: Entered ViewPage" << "\n"; }
void ViewPage::OnExit() { std::cout << "[INFO]: Exitted ViewPage" << "\n"; }

void ViewPage::Update() {
    auto window_flags = DefaultWindowFlags();
    ImGui::Begin("View Data", nullptr, window_flags);
    ImGui::Columns(2);

    bool opened_file = 0;
    if (ImGui::Button("New File")) {
        auto maybe_path = OpenFile();
        if (maybe_path.has_value()) {
            video_path = maybe_path.value();
            cap.open(video_path);
        }
    }

    if (cap.isOpened()) {
        if (cap.read(frame)) {
            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
            //Upload to texture helper
            ImGui::Image((void*)(intptr_t)video_texture, ImVec2{frame.cols, frame.rows});
        }
    }

    ImGui::End();
}

std::optional<std::string> ViewPage::OpenFile() {
    const char* filters[] = {"*.mp4", "*.avi", "*.mov"};
    std::string path      = tinyfd_openFileDialog("Select a video file", "", 3, filters, NULL, 0);
    if (path.empty()) {
        std::cout << "No file selected\n";
        return std::nullopt;
    } else {
        return path;
    }
}
