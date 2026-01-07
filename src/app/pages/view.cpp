#include <algorithm>
#include <cassert>
#include <cstddef>
#include <filesystem>
#include <fstream>

#include <tinyfiledialogs.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>

#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>

#include "core/log.hpp"

#include "app/assets/images/image_buttons.hpp"
#include "app/common/plot.hpp"
#include "app/common/scope.hpp"
#include "app/common/text.hpp"
#include "app/pages/view.hpp"
#include "app/style.hpp"

using namespace std::chrono_literals;

static constexpr size_t MAX_QUEUE_SIZE = 10;

const char* ViewPage::DataTypeString(DataView type) {
    switch (type) {
    case DataView::ALL:
        return "All Data Shown";
    case DataView::RPMDATA:
        return "RPM Data Shown";
    case DataView::SHOCKDATA:
        return "Shock Data Shown";
    default:
        return "Unknown";
    }
}

ViewPage::ViewPage(const std::shared_ptr<AppContext>& ctx)
    : Page{ctx}, m_IsAlive{std::make_shared<bool>(true)},
      m_PlayButton{PLAY_BUTTON_PNG, PLAY_BUTTON_PNG_SIZE},
      m_PauseButton{PAUSE_BUTTON_PNG, PAUSE_BUTTON_PNG_SIZE} {}

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
    if (const ImGuiScope<ImGui::EndTable> split{IMSCOPE_FN(ImGui::BeginTable(
            "##viewsplt", 2, ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Resizable))}) {
        ImGui::TableNextColumn();
        DrawLHS();
        ImGui::TableNextColumn();
        DrawRHS();
    }
}

void ViewPage::Cleanup() {
    StopDecodingThread();
    TryCleanupSokolResources();
}

void ViewPage::DrawLHS() {
    if (const ImGuiScope<ImGui::EndChild> video_player{IMSCOPE_FN(ImGui::BeginChild("##video"))}) {
        DrawOpenVideo();
        if (m_VideoLoaded && !m_VideoPath.empty()) {
            ImGui::SameLine();
            const std::filesystem::path path{m_VideoPath};
            const auto                  filename_str = path.filename().string();
            BOLD_DEFAULT(ImGui::TextUnformatted("Current File: "));
            ImGui::SameLine();
            ImGui::TextUnformatted(filename_str.c_str());
        }
        ImGui::SameLine();

        // Clear Video button is right aligned
        const char*  label       = "Clear Video";
        const ImVec2 button_size = ImGui::CalcTextSize(label);
        const float  padding     = ImGui::GetStyle().FramePadding.x * 2.0F;
        const float  right_x     = ImGui::GetWindowContentRegionMax().x - (button_size.x + padding);

        ImGui::SetCursorPosX(right_x);
        if (ImGui::Button(label)) {
            StopDecodingThread();
            m_IsPlaying = false;
            m_VideoPath = {};
            TryCleanupSokolResources();
            return;
        }

        ImGui::Separator();

        // Playback logic and frame skipping can be ignored if paused
        bool is_timer_tick = false;
        if (m_IsPlaying && m_VideoFPS > 0.0F) {
            m_TimeAccumulator += ImGui::GetIO().DeltaTime;
            const auto frames_to_advance = static_cast<int>(m_TimeAccumulator / m_FrameDuration);

            // We have to handle skipped frames gracefully
            if (frames_to_advance > 0) {
                m_TimeAccumulator -= (frames_to_advance * m_FrameDuration);

                if (frames_to_advance > 1) {
                    const std::scoped_lock<std::mutex> lock{m_FrameMutex};

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
            const float aspect  = static_cast<float>(m_TexWidth) / static_cast<float>(m_TexHeight);
            const float avail_w = ImGui::GetContentRegionAvail().x;
            const float h       = avail_w / aspect;

            ImGui::Image(m_VideoTextureID, {avail_w, h});
            m_VideoHovered = ImGui::IsItemHovered();
        }

        if (m_ThreadRunning) {
            ImGui::Separator();
            DrawLHSControls();
        }
    }
}

void ViewPage::DrawLHSControls() {
    // Slider
    const auto current_timestamp_min =
        (static_cast<double>(m_CurrentFrameUI) / m_TotalFrames) * m_VideoLengthMin;
    const auto current_timestamp   = LocalTime::FromMinutes(current_timestamp_min);
    const auto formatted_timestamp = current_timestamp.value_or(LocalTime::Zero()).String(false);
    ImGui::Text("%s / %s", formatted_timestamp.c_str(), m_VideoLengthFormatted.c_str());
    ImGui::SameLine();

    if (const ImGuiScope<ImGui::PopItemWidth> slider_width{IMSCOPE_FN(ImGui::PushItemWidth(-1))}) {
        int slider_pos = m_CurrentFrameUI;
        if (ImGui::SliderInt(
                "##scrub", &slider_pos, 0, m_TotalFrames, "", ImGuiSliderFlags_NoInput)) {
            m_IsPlaying.exchange(false);
            RequestSeek(slider_pos);
        }
    }

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

    // Keyboard shortcuts
    if (m_VideoHovered && !m_TimestampInputFocused && !m_Context->CommandInputFocused) {
        if (ImGui::IsKeyPressed(ImGuiKey_Space)) {
            m_IsPlaying       = !m_IsPlaying;
            m_TimeAccumulator = 0.0;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow, false)) {
            RequestSeek(m_CurrentFrameUI - 1);
            m_IsPlaying.exchange(false);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow, false)) {
            RequestSeek(m_CurrentFrameUI - 10);
            m_IsPlaying.exchange(false);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow, false)) {
            RequestSeek(m_CurrentFrameUI + 1);
            m_IsPlaying.exchange(false);
        }

        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow, false)) {
            RequestSeek(m_CurrentFrameUI + 10);
            m_IsPlaying.exchange(false);
        }
    }
}

void ViewPage::DrawOpenVideo() {
    // Check if the file is ready
    {
        const std::scoped_lock<std::mutex> lock{m_VideoPathMutex};
        if (m_SelectedVideo) {
            StopDecodingThread();
            TryCleanupSokolResources();
            m_VideoPath     = m_SelectedVideo.value().first;
            m_SelectedVideo = std::nullopt;
            m_VideoLoaded   = true;
            StartDecodingThread();
        }
    }

    // File selection can happen at any point during playback
    const bool is_disabled_by_video = m_VideoDialogRunning.load();
    if (is_disabled_by_video) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Open Video")) {
        m_VideoDialogRunning = true;
        m_VideoLoaded        = false;
        m_DynamicPlotting    = false;

        const auto  alive         = m_IsAlive;
        const auto& previous_file = m_VideoPath;

        std::thread([this, alive, previous_file]() noexcept {
            try {
                const auto path = OpenVideoFile(previous_file);
                if (*alive) {
                    const std::scoped_lock<std::mutex> lock{m_VideoPathMutex};
                    m_SelectedVideo      = path;
                    m_VideoDialogRunning = false;
                }
            } catch (const std::exception& e) {
                m_VideoDialogRunning = false;
                LOG_CRITICAL("Fatal error occurred while opening video dialog: {}", e.what());
            } catch (...) {
                m_VideoDialogRunning = false;
                LOG_CRITICAL("Unknown fatal error occurred");
            }
        }).detach();
    }
    if (is_disabled_by_video) { ImGui::EndDisabled(); }
}

void ViewPage::DrawRHS() {
    if (const ImGuiScope<ImGui::EndChild> data_child{IMSCOPE_FN(ImGui::BeginChild("##data"))}) {
        HEADER({
            DrawOpenText();
            ImGui::SameLine();
            DrawSyncVideoButtons();
            ImGui::Separator();
        });

        if (const ImGuiScope<ImGui::EndCombo, REQUIRE_ALIVE_FOR_DTOR> combo{
                IMSCOPE_FN(ImGui::BeginCombo("##DataView", DataTypeString(m_DataShow)))}) {
            if (ImGui::Selectable("All Data", m_DataShow == DataView::ALL)) {
                m_DataShow = DataView::ALL;
            }
            if (ImGui::Selectable("RPM", m_DataShow == DataView::RPMDATA)) {
                m_DataShow = DataView::RPMDATA;
            }
            if (ImGui::Selectable("Shock", m_DataShow == DataView::SHOCKDATA)) {
                m_DataShow = DataView::SHOCKDATA;
            }
        }

        const auto  data  = m_Context->Backend->PackData();
        const auto& rpm   = data.RPM;
        const auto& shock = data.Shock;

        const auto sync_lt = m_Context->Backend->Data.GetSyncLT();
        const auto plot_title =
            sync_lt ? std::format("Data View from {}", sync_lt.value().String()) : "No Synced Time";

        ViewPage::DynamicPlotLoop();
        if (const ImGuiScope<ImPlot::EndPlot, REQUIRE_ALIVE_FOR_DTOR> plot{
                IMSCOPE_FN(ImPlot::BeginPlot(plot_title.c_str(), {-1, -1}))}) {
            PlotUtils::PlotIfNonEmpty("Wheel Speed",
                                      data.TimeMinutesNormalized,
                                      rpm.WheelRPM,
                                      m_DataShow == DataView::ALL ||
                                          m_DataShow == DataView::RPMDATA,
                                      m_PlotPercent);
            PlotUtils::PlotIfNonEmpty("Engine Speed",
                                      data.TimeMinutesNormalized,
                                      rpm.EngineRPM,
                                      m_DataShow == DataView::ALL ||
                                          m_DataShow == DataView::RPMDATA,
                                      m_PlotPercent);

            PlotUtils::PlotIfNonEmpty("Front Right Shock Travel",
                                      data.TimeMinutesNormalized,
                                      shock.FrontRight,
                                      m_DataShow == DataView::ALL ||
                                          m_DataShow == DataView::SHOCKDATA,
                                      m_PlotPercent);
            PlotUtils::PlotIfNonEmpty("Front Left Shock Travel",
                                      data.TimeMinutesNormalized,
                                      shock.FrontLeft,
                                      m_DataShow == DataView::ALL ||
                                          m_DataShow == DataView::SHOCKDATA,
                                      m_PlotPercent);
            PlotUtils::PlotIfNonEmpty("Rear Right Shock Travel",
                                      data.TimeMinutesNormalized,
                                      shock.BackRight,
                                      m_DataShow == DataView::ALL ||
                                          m_DataShow == DataView::SHOCKDATA,
                                      m_PlotPercent);
            PlotUtils::PlotIfNonEmpty("Rear Left Shock Travel",
                                      data.TimeMinutesNormalized,
                                      shock.BackLeft,
                                      m_DataShow == DataView::ALL ||
                                          m_DataShow == DataView::SHOCKDATA,
                                      m_PlotPercent);
        }
    }
}

void ViewPage::DrawOpenText() {
    // Check if the file is ready
    {
        const std::scoped_lock<std::mutex> lock{m_TxtPathMutex};
        if (m_SelectedTxt) {
            m_TxtPath     = m_SelectedTxt.value();
            m_SelectedTxt = std::nullopt;
            m_TxtLoaded   = true;
            LoadData();
        }
    }

    // File selection can happen at any point during playback
    const bool is_disabled_by_txt = m_TxtDialogRunning.load();
    if (is_disabled_by_txt) { ImGui::BeginDisabled(); }
    if (ImGui::Button("Open Text File")) {
        m_TxtDialogRunning = true;
        m_TxtLoaded        = false;
        m_DynamicPlotting  = false;

        const auto alive         = m_IsAlive;
        const auto previous_path = m_TxtPath;

        std::thread([this, alive, previous_path]() noexcept {
            try {
                const auto path = OpenTextFile(previous_path);
                if (*alive) {
                    const std::scoped_lock<std::mutex> lock{m_TxtPathMutex};
                    m_SelectedTxt                 = path;
                    m_TxtDialogRunning            = false;
                    m_Context->Backend->IsLogging = false;
                }
            } catch (const std::exception& e) {
                m_TxtDialogRunning = false;
                LOG_CRITICAL("Fatal error occurred while opening text file dialog: {}", e.what());
            } catch (...) {
                m_TxtDialogRunning = false;
                LOG_CRITICAL("Unknown fatal error occurred");
            }
        }).detach();
    }
    if (is_disabled_by_txt) { ImGui::EndDisabled(); }
}

void ViewPage::DrawSyncVideoButtons() {
    if (ImGui::Button("Sync Data/Video")) {
        m_DataAndTimeSync = false;
        m_DynamicPlotting = false;

        LoadData();
        if (!m_TxtLoaded || !m_VideoLoaded) {
            LOG_ERROR("Could not sync data with video:");
            LOG_ERROR("  Text Loaded: {}", m_TxtLoaded);
            LOG_ERROR("  Video Loaded: {}", m_VideoLoaded);
            return;
        }

        m_VideoCreationTimestamp = LocalTime::FromString(m_CreationMetadataTextBuf);
        if (!m_VideoCreationTimestamp) {
            LOG_ERROR("Could not parse provided timestamp, or it was not provided.");
            return;
        }
        m_CreationMetadataTextBuf = {};

        std::optional<size_t> sync_time_pos;
        {
            const std::scoped_lock<std::mutex> lock{m_Context->Backend->DataMutex};
            sync_time_pos = SyncDataVideo(m_Context->Backend->Data.GetTimeNoNormal());
        }

        if (!sync_time_pos) {
            LOG_ERROR("Could not get trim position from data/video");
            return;
        }

        // This is probably mega unsafe...
        DeleteExtra(sync_time_pos.value());
        RequestSeek(0);
    }

    ImGui::SameLine();
    TextUtils::DrawInputBox("##extra_view", m_CreationMetadataTextBuf, "HH:MM:SS", 120.0F);
    m_TimestampInputFocused = ImGui::IsItemFocused();
    ImGui::SameLine();
    if (ImGui::Checkbox("Dynamic Plotting", &m_DynamicPlotting)) { ViewPage::DynamicPlotStart(); }
}

ViewPage::SelectedVideo ViewPage::OpenVideoFile(const std::string& previous_file) {
    const char* const filters[] = {"*.mp4", "*.mov"};
    const char*       path      = tinyfd_openFileDialog(
        "Select a video file", previous_file.c_str(), std::size(filters), filters, nullptr, 0);
    if (path == nullptr) {
        LOG_WARN("No file selected");
        return std::nullopt;
    }

    const std::string real_path{path};
    LOG_INFO("Selected file: {}", real_path);

    auto dt = DateTime::FromVideoMetadata(real_path);
    if (dt) {
        LOG_INFO("Selected video with creation timestamp: {}", dt.value().String());
    } else {
        LOG_WARN("Could not detect datetime metadata from selected video.");
    }

    return std::pair{path, dt};
}

ViewPage::SelectedTxtFile ViewPage::OpenTextFile(const std::string& previous_file) {
    const char* const filters[] = {"*.txt"};
    const char*       path      = tinyfd_openFileDialog(
        "Select a text file", previous_file.c_str(), std::size(filters), filters, nullptr, 0);
    if (path == nullptr) {
        LOG_WARN("No file selected");
        return std::nullopt;
    }

    LOG_INFO("Selected file: {}", path);
    return path;
}

void ViewPage::LoadData() {
    std::ifstream file{m_TxtPath};
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: {}", m_TxtPath);
        return;
    }

    const std::scoped_lock<std::mutex> lock{m_Context->Backend->DataMutex};
    m_Context->Backend->Data.Clear();
    std::string ident, value;
    while (file >> ident >> value) {
        m_Context->Backend->Data.WriteData(ident, value);
    }
}

void ViewPage::RequestSeek(int frame_index) {
    {
        const std::scoped_lock<std::mutex> lock{m_FrameMutex};
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

        m_VideoFPS                 = cap.get(cv::CAP_PROP_FPS);
        m_FrameDuration            = 1.0 / m_VideoFPS;
        m_TotalFrames              = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));
        m_VideoLengthMin           = (static_cast<double>(m_TotalFrames) / m_VideoFPS) / 60.0;
        const auto video_length_lt = LocalTime::FromMinutes(m_VideoLengthMin);
        if (!video_length_lt) {
            LOG_ERROR("Video length could not be determined");
            m_ThreadRunning = false;
            return;
        }
        m_VideoLengthFormatted = video_length_lt.value().String(false);

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

            const int seek_req    = m_SeekTarget.exchange(-1);
            bool      just_sought = false;

            if (seek_req != -1) {
                cap.set(cv::CAP_PROP_POS_FRAMES, seek_req);
                const std::scoped_lock<std::mutex> lock{m_FrameMutex};
                m_FrameQueue.clear();
                just_sought = true;
            }

            // Don't decode if seeking or paused to prevent jitters
            if (!m_IsPlaying && !just_sought) {
                std::this_thread::sleep_for(5ms);
                continue;
            }

            auto current_frame_index = static_cast<int>(cap.get(cv::CAP_PROP_POS_FRAMES));
            if (cap.read(raw_frame)) {
                cv::cvtColor(raw_frame, rgba_frame, cv::COLOR_BGR2RGBA);
                const cv::Mat frame_copy = rgba_frame.clone();

                const std::scoped_lock<std::mutex> lock{m_FrameMutex};
                m_FrameQueue.emplace_back(frame_copy, current_frame_index);
            } else {
                if (m_IsLooping) {
                    cap.set(cv::CAP_PROP_POS_FRAMES, 0);
                } else {
                    std::this_thread::sleep_for(10ms);
                }
            }
        }
    });
}

void ViewPage::StopDecodingThread() {
    m_ThreadRunning = false;
    m_QueueCV.notify_all();
    if (m_DecodeThread.joinable()) { m_DecodeThread.join(); }

    const std::scoped_lock<std::mutex> lock{m_FrameMutex};
    m_FrameQueue.clear();
}

void ViewPage::UpdateTexture(bool is_timer_tick) {
    cv::Mat frame_to_upload;
    bool    frame_ready = false;
    {
        const std::scoped_lock<std::mutex> lock{m_FrameMutex};
        const bool                         should_consume = is_timer_tick || m_ForceUpdateFrame;

        if (should_consume && !m_FrameQueue.empty()) {
            const auto p     = m_FrameQueue.front();
            frame_to_upload  = p.first;
            m_CurrentFrameUI = p.second;

            m_FrameQueue.pop_front();
            frame_ready = true;
            m_QueueCV.notify_one();
            m_ForceUpdateFrame.exchange(false);
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

std::optional<size_t> ViewPage::SyncDataVideo(const std::vector<uint64_t>& micros_times) {
    if (!m_VideoCreationTimestamp) { return std::nullopt; }

    m_DataAndTimeSync                 = true;
    const auto     creation_timestamp = m_VideoCreationTimestamp.value();
    const uint64_t micros_to_sync     = creation_timestamp.MicrosSinceMidnight();
    const auto     it                 = std::ranges::lower_bound(micros_times, micros_to_sync);
    if (it != micros_times.end()) { return std::distance(micros_times.begin(), it); }

    return std::nullopt;
}

void ViewPage::DeleteExtra(size_t erase_pos) {
    const std::scoped_lock<std::mutex> lock{m_Context->Backend->DataMutex};
    std::vector<double>* const         data[] = {
        &m_Context->Backend->Data.m_Time,
        &m_Context->Backend->Data.m_RPMData.WheelRPM,
        &m_Context->Backend->Data.m_RPMData.EngineRPM,
        &m_Context->Backend->Data.m_ShockData.FrontRight,
        &m_Context->Backend->Data.m_ShockData.FrontLeft,
        &m_Context->Backend->Data.m_ShockData.BackRight,
        &m_Context->Backend->Data.m_ShockData.BackLeft,
    };

    for (const auto& datum : data) {
        if (datum->size() < erase_pos) { return; }
    }

    for (const auto& datum : data) {
        datum->erase(datum->begin(), datum->begin() + static_cast<ptrdiff_t>(erase_pos));
    }
}

void ViewPage::DynamicPlotStart() {
    if (m_DynamicPlotting) {
        const std::scoped_lock<std::mutex> lock{m_Context->Backend->DataMutex};
        const auto&                        time_vec = m_Context->Backend->Data.m_Time;
        if (time_vec.empty()) { return; }
        const auto begin_time_min = m_Context->Backend->Data.m_Time[0];

        const double target_end_time = begin_time_min + m_VideoLengthMin;
        const auto   end_it          = std::ranges::lower_bound(time_vec, target_end_time);

        const size_t end_idx = std::distance(time_vec.begin(), end_it);
        const size_t end_idy = std::distance(end_it, time_vec.end());
        m_DataFromEnd        = static_cast<double>(end_idy);
        m_DataCount          = static_cast<double>(end_idx);
        m_PointsPer          = static_cast<double>(end_idx) / m_TotalFrames;
    }
}

void ViewPage::DynamicPlotLoop() {
    if (m_DynamicPlotting) {
        m_PlotPercent = static_cast<size_t>(std::max(
            m_DataCount - (m_PointsPer * m_CurrentFrameUI) + m_DataFromEnd, m_DataFromEnd));
        return;
    }
    m_PlotPercent = 0;
}
