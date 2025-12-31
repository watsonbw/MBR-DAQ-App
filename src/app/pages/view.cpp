#include <iostream>

#include <tinyfiledialogs.h>

#include <imgui.h>

#include "app/pages/view.hpp"

#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_imgui.h"

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
            if (video_texture.id != SG_INVALID_ID) {
                sg_destroy_image(video_texture);
                video_texture.id = SG_INVALID_ID;
            }
        }
        
    }

    if (cap.isOpened()) {
        if (cap.read(frame)) {

            cv::cvtColor(frame, frame, cv::COLOR_BGR2RGBA);
            CV_Assert(frame.isContinuous());


            if (video_texture.id == SG_INVALID_ID) { 
                sg_image_desc desc = {}; 
                desc.width = frame.cols; 
                desc.height = frame.rows; 
                desc.pixel_format = SG_PIXELFORMAT_RGBA8;
                desc.num_mipmaps = 1;
                video_texture = sg_make_image(&desc); 
            }

            FtoT(frame, video_texture);
            ImGui::Image((ImTextureID)(uintptr_t)video_texture.id, ImVec2{(float)frame.cols, (float)frame.rows});
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

void FtoT(const cv::Mat& frame, sg_image tex) {

    sg_image_data data = {};

    data.mip_levels[0].ptr = frame.data;
    data.mip_levels[0].size = frame.total() * frame.elemSize(); 
    sg_update_image(tex, &data);

};
