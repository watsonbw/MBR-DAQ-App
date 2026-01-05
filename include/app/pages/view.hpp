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

#include "core/time.hpp"

#include "app/assets/texture.hpp"
#include "app/pages/page.hpp"

class ViewPage : public Page {
  public:
    using SelectedVideo   = std::optional<std::pair<std::string, std::optional<DateTime>>>;
    using SelectedTxtFile = std::optional<std::string>;

    enum class DataView {
        ALL,
        RPMDATA,
        SHOCKDATA,
    };

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

    static SelectedVideo OpenVideoFile();
    void                 RequestSeek(int frame_index);

    static SelectedTxtFile OpenTextFile();
    void                   LoadData(const std::string& path);
    const char*            DataTypeString(DataView type);

    void StartDecodingThread();
    void StopDecodingThread();
    void UpdateTexture(bool is_timer_tick);

    void TryCleanupSokolResources();

  private:
    std::shared_ptr<bool> m_IsAlive;

    std::string             m_VideoPath;
    std::optional<DateTime> m_VideoCreationTimestamp;
    std::atomic<bool>       m_VideoDialogRunning{false};
    std::mutex              m_VideoPathMutex;
    SelectedVideo           m_SelectedVideo;

    std::string       m_TxtPath;
    std::atomic<bool> m_TxtDialogRunning{false};
    std::mutex        m_TxtPathMutex;
    SelectedTxtFile   m_SelectedTxt;

    int    m_TotalFrames{0};
    double m_VideoFPS{0.0};
    double m_FrameDuration{0.0};
    double m_TimeAccumulator{0.0};
    int    m_CurrentFrameUI{0};

    std::atomic<bool> m_IsPlaying{false};
    std::atomic<bool> m_IsLooping{false};
    std::atomic<int>  m_SeekTarget{-1};
    std::atomic<bool> m_ForceUpdateFrame{false};
    DataView          m_DataShow{DataView::ALL};

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
