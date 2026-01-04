#include <cassert>
#include <chrono>

#include <tinyfiledialogs.h>

#include <imgui.h>
#include <implot.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>

#include "core/log.hpp"
#include "core/time.hpp"

#include "app/assets/images/image_buttons.hpp"
#include "app/pages/view.hpp"

static constexpr size_t MAX_QUEUE_SIZE = 10;

ViewPage::ViewPage(std::shared_ptr<AppContext> ctx)
    : Page{ctx}, m_IsAlive{std::make_shared<bool>(true)},
      m_PlayButton{PlayButton_png, PlayButton_png_size},
      m_PauseButton{PauseButton_png, PauseButton_png_size} {}

ViewPage::~ViewPage() {
    *m_IsAlive = false;
    Cleanup();
    LOG_INFO("Destroyed ViewPage");
}

void ViewPage::OnEnter() { LOG_INFO("Entered ViewPage"); }

void ViewPage::OnExit() {
    Cleanup();
    LOG_INFO("Exited ViewPage");
}

void ViewPage::Update() {
    auto window_flags = DefaultWindowFlags();
    ImGui::Begin("View Data", nullptr, window_flags);

    ImGui::Columns(2);
    DrawLHS();
    ImGui::NextColumn();
    DrawRHS();

    ImGui::End();
}

void ViewPage::Cleanup() {
    StopDecodingThread();
    TryCleanupSokolResources();
}

void ViewPage::DrawLHS() {
    ImGui::BeginChild("VideoPlayerColumn");

    // Check if the file is ready
    {
        std::lock_guard<std::mutex> lock{m_PathMutex};
        if (m_SelectedPath.has_value()) {
            StopDecodingThread();
            TryCleanupSokolResources();
            m_VideoPath    = m_SelectedPath.value();
            m_SelectedPath = std::nullopt;
            StartDecodingThread();
        }
    }

    // File selection can happen at any point during playback
    const bool is_disabled = m_DialogRunning.load();
    if (is_disabled) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Open Video")) {
        m_DialogRunning  = true;
        const auto alive = m_IsAlive;

        std::thread([this, alive]() {
            const auto path = OpenFile();
            if (*alive) {
                std::lock_guard<std::mutex> lock{m_PathMutex};
                m_SelectedPath  = path;
                m_DialogRunning = false;
            }
        }).detach();
    }
    if (is_disabled) { ImGui::EndDisabled(); }

    // Playback logic and frame skipping can be ignored if paused
    bool is_timer_tick = false;
    if (m_IsPlaying && m_VideoFPS > 0.0f) {
        m_TimeAccumulator += ImGui::GetIO().DeltaTime;
        const auto frames_to_advance = static_cast<int>(m_TimeAccumulator / m_FrameDuration);

        // We have to handle skipped frames gracefully
        if (frames_to_advance > 0) {
            m_TimeAccumulator -= (frames_to_advance * m_FrameDuration);

            if (frames_to_advance > 1) {
                std::lock_guard<std::mutex> lock{m_FrameMutex};

                const auto recoverable_frames =
                    std::min(static_cast<int>(m_FrameQueue.size()) - 1, frames_to_advance - 1);
                for (auto i = 0; i < recoverable_frames; i++) {
                    m_FrameQueue.pop_front();
                }
            }

            is_timer_tick = true;
        }
    }

    UpdateTexture(is_timer_tick);

    if (m_VideoTexture.id != SG_INVALID_ID && m_TexWidth > 0) {
        float aspect  = (float)m_TexWidth / (float)m_TexHeight;
        float avail_w = ImGui::GetContentRegionAvail().x;
        float h       = avail_w / aspect;

        ImGui::Image(m_VideoTextureID, ImVec2(avail_w, h));
    }

    if (m_ThreadRunning) { DrawLHSControls(); }

    ImGui::EndChild();
}

void ViewPage::DrawLHSControls() {
    ImGui::Separator();

    // Slider
    int slider_pos = m_CurrentFrameUI;
    ImGui::PushItemWidth(-1);
    if (ImGui::SliderInt("##scrub", &slider_pos, 0, m_TotalFrames)) {
        if (m_IsPlaying) { m_IsPlaying = false; }
        RequestSeek(slider_pos);
    }
    ImGui::PopItemWidth();

    // Loop checkbox
    bool looping = m_IsLooping;
    if (ImGui::Checkbox("Looping", &looping)) { m_IsLooping = looping; }
    ImGui::SameLine();

    // Play/Pause
    if (ImGui::ImageButton("PlayPause",
                           m_IsPlaying ? m_PauseButton.GetID() : m_PlayButton.GetID(),
                           m_ButtonSize)) {
        m_IsPlaying       = !m_IsPlaying;
        m_TimeAccumulator = 0.0;
    }
}

void ViewPage::DrawRHS() {
    ImGui::BeginChild("Data");

    std::vector<double> time, fr, fl, br, bl;

    {
        std::lock_guard lock{m_Context->Backend->DataMutex};
        time = m_Context->Backend->Data.GetTime();

        const auto& shock_data = m_Context->Backend->Data.GetShockData();
        fr                     = shock_data.FrontRight;
        fl                     = shock_data.FrontLeft;
        br                     = shock_data.BackRight;
        bl                     = shock_data.BackRight;
    }

    if (ImPlot::BeginPlot("RPM Over Time", {-1, -1})) {
        if (!time.empty()) {
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), fr.data(), time.size());
            }
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), fl.data(), time.size());
            }
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), br.data(), time.size());
            }
            if (!fr.empty()) {
                ImPlot::PlotLine("Wheel Speed", time.data(), bl.data(), time.size());
            }
        }
        ImPlot::EndPlot();
    }

    ImGui::EndChild();
}

std::optional<std::string> ViewPage::OpenFile() {
    const char* filters[] = {"*.mp4", "*.mov"};
    const char* path =
        tinyfd_openFileDialog("Select a video file", "", std::size(filters), filters, NULL, 0);
    if (path == nullptr) {
        LOG_WARN("No file selected");
        return std::nullopt;
    }

    const std::string real_path{path};
    LOG_INFO("Selected file: {}", real_path);

    auto dt = DateTime::FromVideoMetadata(real_path);
    if (dt.has_value()) {
        LOG_INFO("Selected video with creation timestamp: {}", dt.value().String());
    } else {
        LOG_WARN("Could not detect datetime metadata from selected video.");
    }

    return path;
}

void ViewPage::RequestSeek(int frame_index) {
    {
        std::lock_guard<std::mutex> lock{m_FrameMutex};
        m_FrameQueue.clear();
    }

    m_SeekTarget       = frame_index;
    m_ForceUpdateFrame = true;
    m_CurrentFrameUI   = frame_index;
    m_QueueCV.notify_one();
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

        m_VideoFPS      = cap.get(cv::CAP_PROP_FPS);
        m_FrameDuration = 1.0 / m_VideoFPS;
        m_TotalFrames   = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

        cv::Mat raw_frame, rgba_frame;
        while (m_ThreadRunning) {
            // Keep the buffer from becoming too large
            {
                std::unique_lock<std::mutex> lock{m_FrameMutex};
                m_QueueCV.wait(lock, [this] {
                    return m_FrameQueue.size() < MAX_QUEUE_SIZE || !m_ThreadRunning;
                });

                if (!m_ThreadRunning) { break; }
            }

            int  seek_req    = m_SeekTarget.exchange(-1);
            bool just_sought = false;

            if (seek_req != -1) {
                cap.set(cv::CAP_PROP_POS_FRAMES, seek_req);
                std::lock_guard<std::mutex> lock{m_FrameMutex};
                m_FrameQueue.clear();
                just_sought = true;
            }

            // Don't decode if seeking or paused to prevent jitters
            if (!m_IsPlaying && !just_sought) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
                continue;
            }

            auto current_frame_index = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
            if (cap.read(raw_frame)) {
                cv::cvtColor(raw_frame, rgba_frame, cv::COLOR_BGR2RGBA);
                cv::Mat frame_copy = rgba_frame.clone();

                std::lock_guard<std::mutex> lock{m_FrameMutex};
                m_FrameQueue.push_back({frame_copy, current_frame_index});
            } else {
                if (m_IsLooping) {
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                } else {
                    m_IsPlaying = false;
                }
            }
        }
    });
}

void ViewPage::StopDecodingThread() {
    m_ThreadRunning = false;
    m_QueueCV.notify_all();
    if (m_DecodeThread.joinable()) { m_DecodeThread.join(); }

    std::lock_guard<std::mutex> lock{m_FrameMutex};
    m_FrameQueue.clear();
}

void ViewPage::UpdateTexture(bool is_timer_tick) {
    cv::Mat frame_to_upload;
    bool    frame_ready = false;
    {
        std::lock_guard<std::mutex> lock{m_FrameMutex};
        bool                        should_consume = is_timer_tick || m_ForceUpdateFrame;

        if (should_consume && !m_FrameQueue.empty()) {
            const auto p     = m_FrameQueue.front();
            frame_to_upload  = p.first;
            m_CurrentFrameUI = p.second;

            m_FrameQueue.pop_front();
            frame_ready = true;
            m_QueueCV.notify_one();

            if (m_ForceUpdateFrame) { m_ForceUpdateFrame = false; }
        }
    }

    if (frame_ready && !frame_to_upload.empty()) {
        // Initialize Texture if resolution changed or first run
        if (m_TexWidth != frame_to_upload.cols || m_TexHeight != frame_to_upload.rows) {
            TryCleanupSokolResources();

            m_TexWidth  = frame_to_upload.cols;
            m_TexHeight = frame_to_upload.rows;

            sg_image_desc desc       = {};
            desc.width               = m_TexWidth;
            desc.height              = m_TexHeight;
            desc.pixel_format        = SG_PIXELFORMAT_RGBA8;
            desc.usage.stream_update = true;
            desc.num_mipmaps         = 1;
            m_VideoTexture           = sg_make_image(&desc);
            assert(m_VideoTexture.id != SG_INVALID_ID);

            sg_view_desc view_desc  = {};
            view_desc.texture.image = m_VideoTexture;
            m_VideoView             = sg_make_view(&view_desc);
            m_VideoTextureID        = simgui_imtextureid(m_VideoView);
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

    m_TexWidth  = 0;
    m_TexHeight = 0;
}
