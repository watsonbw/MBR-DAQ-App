#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <imgui.h>

#include <opencv2/opencv.hpp>

#include <sokol_gfx.h>

#include "app/assets/texture.hpp"
#include "app/pages/page.hpp"

class ViewPage : public Page {
  public:
    ViewPage(std::shared_ptr<AppContext> ctx);
    virtual ~ViewPage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;

  private:
    void DrawLHS();
    void DrawLHSControls();
    void DrawRHS();

    std::optional<std::string> OpenFile();
    void                       RequestSeek(int frame_index);

    void StartDecodingThread();
    void StopDecodingThread();
    void UpdateTexture(bool is_timer_tick);

    void TryCleanupSokolResources();

  private:
    std::string m_VideoPath;
    int         m_TotalFrames     = 0;
    double      m_VideoFPS        = 0.0;
    double      m_FrameDuration   = 0.0;
    double      m_TimeAccumulator = 0.0;
    int         m_CurrentFrameUI  = 0;

    std::atomic<bool> m_IsPlaying        = false;
    std::atomic<bool> m_IsLooping        = false;
    std::atomic<int>  m_SeekTarget       = -1;
    std::atomic<bool> m_ForceUpdateFrame = false;

    std::thread                         m_DecodeThread;
    std::atomic<bool>                   m_ThreadRunning = false;
    std::mutex                          m_FrameMutex;
    std::deque<std::pair<cv::Mat, int>> m_FrameQueue;
    std::condition_variable             m_QueueCV;
    const size_t                        MAX_QUEUE_SIZE = 10;

    ImVec2        m_ButtonSize = {24, 24};
    ButtonTexture m_PlayButton;
    ButtonTexture m_PauseButton;

    sg_image    m_VideoTexture   = {SG_INVALID_ID};
    sg_view     m_VideoView      = {SG_INVALID_ID};
    ImTextureID m_VideoTextureID = 0;
    int         m_TexWidth       = 0;
    int         m_TexHeight      = 0;
};
