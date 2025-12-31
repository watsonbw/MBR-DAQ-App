#include <tinyfiledialogs.h>

#include <imgui.h>

#include "core/log.hpp"

#include "app/pages/view.hpp"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_imgui.h"

void ViewPage::OnEnter() { LOG_INFO("Entered ViewPage"); }
void ViewPage::OnExit() { LOG_INFO("Exitted ViewPage"); }

void ViewPage::Update() {
    auto window_flags = DefaultWindowFlags();
    ImGui::Begin("View Data", nullptr, window_flags);
    ImGui::Columns(2);

    [[maybe_unused]] bool opened_file = 0;
    if (ImGui::Button("New File")) {
        auto maybe_path = OpenFile();
        if (maybe_path.has_value()) {
            m_VideoPath = maybe_path.value();
            m_Cap.open(m_VideoPath);
            if (m_VideoTexture.id != SG_INVALID_ID) {
                sg_destroy_image(m_VideoTexture);
                m_VideoTexture.id = SG_INVALID_ID;
            }
        }
    }

    if (m_Cap.isOpened()) {
        if (m_Cap.read(m_Frame)) {
            cv::cvtColor(m_Frame, m_Frame, cv::COLOR_BGR2RGBA);
            CV_Assert(m_Frame.isContinuous());

            sg_image_usage usage = {};
            usage.stream_update  = true;
            usage.immutable      = false;

            if (m_VideoTexture.id == SG_INVALID_ID) {
                sg_image_desc desc = {};
                desc.width         = m_Frame.cols;
                desc.height        = m_Frame.rows;
                desc.pixel_format  = SG_PIXELFORMAT_RGBA8;
                desc.usage = usage, desc.num_mipmaps = 1;
                m_VideoTexture = sg_make_image(&desc);
            }

            FrameToTexture(m_Frame, m_VideoTexture);
            ImGui::Image((ImTextureID)(uintptr_t)m_VideoTexture.id,
                         ImVec2{(float)m_Frame.cols, (float)m_Frame.rows});
        }
    }

    ImGui::End();
}

std::optional<std::string> ViewPage::OpenFile() {
    const char* filters[] = {"*.mp4", "*.avi", "*.mov"};
    std::string path      = tinyfd_openFileDialog("Select a video file", "", 3, filters, NULL, 0);
    if (path.empty()) {
        LOG_WARN("No file selected");
        return std::nullopt;
    } else {
        return path;
    }
}

void FrameToTexture(const cv::Mat& frame, sg_image tex) {
    sg_image_data data = {};

    data.mip_levels[0].ptr  = frame.data;
    data.mip_levels[0].size = frame.total() * frame.elemSize();
    sg_update_image(tex, &data);
};
