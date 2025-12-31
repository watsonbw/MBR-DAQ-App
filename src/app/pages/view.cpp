#include <cassert>

#include <tinyfiledialogs.h>

#include <imgui.h>

#include "core/log.hpp"

#include "app/pages/view.hpp"

#include "sokol_app.h"
#include "sokol_gfx.h"
#include "sokol_glue.h"
#include "sokol_imgui.h"

void ViewPage::OnEnter() { LOG_INFO("Entered ViewPage"); }

void ViewPage::OnExit() {
    TryCleanupSokolResources();
    LOG_INFO("Exitted ViewPage");
}

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

            m_VideoFPS        = m_Cap.get(cv::CAP_PROP_FPS);
            m_VideoFPS        = m_VideoFPS <= 0.0 ? 30.0 : m_VideoFPS;
            m_TimeAccumulator = 0.0;

            TryCleanupSokolResources();
        } else {
            ImGui::End();
            return;
        }
    }

    if (!m_VideoPath.empty() && m_Cap.isOpened()) {
        m_TimeAccumulator += ImGui::GetIO().DeltaTime;
        double frame_dur = 1.0 / m_VideoFPS;

        bool did_read_new_frame = false;
        if (m_TimeAccumulator > frame_dur * 2.0) { m_TimeAccumulator = frame_dur; }

        if (m_TimeAccumulator >= frame_dur) {
            m_TimeAccumulator -= frame_dur;
            if (m_Cap.grab() && m_Cap.retrieve(m_RawFrame)) { did_read_new_frame = true; }
        }

        if (did_read_new_frame && !m_RawFrame.empty()) {
            cv::cvtColor(m_RawFrame, m_DisplayFrame, cv::COLOR_BGR2RGBA);
            assert(m_DisplayFrame.isContinuous());

            if (m_VideoTexture.id == SG_INVALID_ID) {
                sg_image_desc desc       = {};
                desc.width               = m_DisplayFrame.cols;
                desc.height              = m_DisplayFrame.rows;
                desc.pixel_format        = SG_PIXELFORMAT_RGBA8;
                desc.usage.stream_update = true;
                desc.num_mipmaps         = 1;
                m_VideoTexture           = sg_make_image(&desc);

                if (m_VideoTexture.id != SG_INVALID_ID) {
                    sg_view_desc view_desc  = {};
                    view_desc.texture.image = m_VideoTexture;

                    m_VideoView      = sg_make_view(&view_desc);
                    m_VideoTextureID = simgui_imtextureid(m_VideoView);
                }
            }

            UpdateFrameToTexture();
        }

        if (m_VideoTexture.id != SG_INVALID_ID && m_DisplayFrame.cols > 0 &&
            m_DisplayFrame.rows > 0) {
            ImGui::Image((ImTextureID)(uintptr_t)m_VideoTextureID,
                         ImVec2{(float)m_DisplayFrame.cols, (float)m_DisplayFrame.rows});
        }
    }

    ImGui::End();
}

std::optional<std::string> ViewPage::OpenFile() {
    const char* filters[] = {"*.mp4", "*.avi", "*.mov"};
    const char* path      = tinyfd_openFileDialog("Select a video file", "", 3, filters, NULL, 0);
    if (path == nullptr) {
        LOG_WARN("No file selected");
        return std::nullopt;
    }

    return path;
}

void ViewPage::UpdateFrameToTexture() {
    sg_image_data data = {};

    data.mip_levels[0].ptr  = m_DisplayFrame.data;
    data.mip_levels[0].size = m_DisplayFrame.total() * m_DisplayFrame.elemSize();
    sg_update_image(m_VideoTexture, &data);
};

void ViewPage::TryCleanupSokolResources() {
    if (m_VideoView.id != SG_INVALID_ID) {
        sg_destroy_view(m_VideoView);
        m_VideoView.id = SG_INVALID_ID;
    }

    if (m_VideoTexture.id != SG_INVALID_ID) {
        sg_destroy_image(m_VideoTexture);
        m_VideoTexture.id = SG_INVALID_ID;
        m_VideoTextureID  = 0;
    }
}
