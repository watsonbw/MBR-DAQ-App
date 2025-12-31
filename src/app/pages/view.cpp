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
    StopDecodingThread();
    TryCleanupSokolResources();
    LOG_INFO("Exitted ViewPage");
}

void ViewPage::Update() {
    auto window_flags = DefaultWindowFlags();
    ImGui::Begin("View Data", nullptr, window_flags);

    if (ImGui::Button("Open Video")) {
        auto maybe_path = OpenFile();
        if (maybe_path.has_value()) {
            StopDecodingThread();
            TryCleanupSokolResources();
            m_VideoPath = maybe_path.value();
            StartDecodingThread();
        }
    }

    if (m_IsPlaying && m_VideoFPS > 0.0f) {
        m_TimeAccumulator += ImGui::GetIO().DeltaTime;
        double frame_dur = 1.0 / m_VideoFPS;

        if (m_TimeAccumulator >= frame_dur) {
            m_TimeAccumulator -= frame_dur;
            if (m_TimeAccumulator > frame_dur) { m_TimeAccumulator = 0.0; }

            UpdateTexture();
        }
    }
    UpdateTexture();

    if (m_VideoTexture.id != SG_INVALID_ID && m_TexWidth > 0) {
        float aspect  = (float)m_TexWidth / (float)m_TexHeight;
        float avail_w = ImGui::GetContentRegionAvail().x;
        float h       = avail_w / aspect;

        ImGui::Image(m_VideoTextureID, ImVec2(avail_w, h));
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

void ViewPage::StartDecodingThread() {
    m_ThreadRunning = true;
    m_IsPlaying     = true;

    m_DecodeThread = std::thread([this]() {
        cv::VideoCapture cap{m_VideoPath};
        if (!cap.isOpened()) {
            LOG_ERROR("Failed to open video");
            m_ThreadRunning = false;
            return;
        }

        m_VideoFPS          = cap.get(cv::CAP_PROP_FPS);
        m_TotalFrames       = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

        cv::Mat raw_frame, rgba_frame;
        while (m_ThreadRunning) {
            // Keep the buffer from becoming too large
            {
                std::unique_lock<std::mutex> lock(m_FrameMutex);
                if (m_FrameQueue.size() >= MAX_QUEUE_SIZE) {
                    lock.unlock();
                    std::this_thread::sleep_for(std::chrono::milliseconds(5));
                    continue;
                }
            }

            // Manage the actual frame decoding and buffering
            if (cap.read(raw_frame)) {
                cv::cvtColor(raw_frame, rgba_frame, cv::COLOR_BGR2RGBA);
                cv::Mat frame_copy = rgba_frame.clone();

                {
                    std::lock_guard<std::mutex> lock(m_FrameMutex);
                    m_FrameQueue.push_back(frame_copy);
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    });
}

void ViewPage::StopDecodingThread() {
    m_ThreadRunning = false;
    if (m_DecodeThread.joinable()) { m_DecodeThread.join(); }

    std::lock_guard<std::mutex> lock(m_FrameMutex);
    m_FrameQueue.clear();
}

void ViewPage::UpdateTexture() {
    cv::Mat frame_to_upload;
    bool    frame_ready = false;
    {
        std::lock_guard<std::mutex> lock(m_FrameMutex);
        if (!m_FrameQueue.empty()) {
            if (m_IsPlaying || m_FrameQueue.size() == 1) {
                frame_to_upload = m_FrameQueue.front();
                m_FrameQueue.pop_front();
                frame_ready = true;
            }
        }
    }

    if (frame_ready && !frame_to_upload.empty()) {
        // Initialize Texture if resolution changed or first run
        if (m_TexWidth != frame_to_upload.cols || m_TexHeight != frame_to_upload.rows) {
            TryCleanupSokolResources();

            m_TexWidth = frame_to_upload.cols;
            m_TexHeight = frame_to_upload.rows;

            sg_image_desc desc       = {};
            desc.width               = m_TexWidth;
            desc.height              = m_TexHeight;
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

        // Perform the actual upload
        sg_image_data data      = {};
        data.mip_levels[0].ptr  = frame_to_upload.data;
        data.mip_levels[0].size = frame_to_upload.total() * frame_to_upload.elemSize();
        sg_update_image(m_VideoTexture, &data);
    }
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
