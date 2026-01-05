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


enum DataView {
    ALL,
    RPMDATA,
    SHOCKDATA,
};

class ViewPage : public Page {
  public:
    ViewPage(std::shared_ptr<AppContext> ctx);
    virtual ~ViewPage();

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;

  private:
    void Cleanup();

    void DrawLHS();
    void DrawLHSControls();
    void DrawRHS();

    static std::optional<std::string> OpenVideoFile();
    static std::optional<std::string> OpenTextFile();
    void                              RequestSeek(int frame_index);
    void                              LoadData(std::string path);

    void StartDecodingThread();
    void StopDecodingThread();
    void UpdateTexture(bool is_timer_tick);

    void TryCleanupSokolResources();

    const char* DataTypeString(DataView type);

  private:
    std::string                m_VideoPath;
    std::atomic<bool>          m_DialogRunning{false};
    std::mutex                 m_PathMutex;
    std::optional<std::string> m_SelectedPath;
    std::shared_ptr<bool>      m_IsAlive;
    std::string                m_OpenPath;

    int    m_TotalFrames{0};
    double m_VideoFPS{0.0};
    double m_FrameDuration{0.0};
    double m_TimeAccumulator{0.0};
    int    m_CurrentFrameUI{0};

    std::atomic<bool> m_IsPlaying{false};
    std::atomic<bool> m_IsLooping{false};
    std::atomic<int>  m_SeekTarget{-1};
    std::atomic<bool> m_ForceUpdateFrame{false};
    DataView          m_DataShow{ALL};

    std::thread                         m_DecodeThread;
    std::atomic<bool>                   m_ThreadRunning{false};
    std::mutex                          m_FrameMutex;
    std::deque<std::pair<cv::Mat, int>> m_FrameQueue;
    std::condition_variable             m_QueueCV;

    ImVec2        m_ButtonSize{24, 24};
    ButtonTexture m_PlayButton;
    ButtonTexture m_PauseButton;

    sg_image    m_VideoTexture{SG_INVALID_ID};
    sg_view     m_VideoView{SG_INVALID_ID};
    ImTextureID m_VideoTextureID{0};
    int         m_TexWidth{0};
    int         m_TexHeight{0};
};
