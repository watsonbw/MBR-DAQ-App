#pragma once

#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

#include <opencv2/opencv.hpp>

#include "app/pages/page.hpp"

#include "sokol_gfx.h"

class ViewPage : public Page {
  public:
    ViewPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ViewPage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;

  private:
    std::optional<std::string> OpenFile();

    void StartDecodingThread();
    void StopDecodingThread();
    void UpdateTexture();

    void TryCleanupSokolResources();

  private:
    std::string      m_VideoPath;
    cv::VideoCapture m_Cap;
    cv::Mat          m_RawFrame;
    cv::Mat          m_DisplayFrame;
    int              m_TotalFrames = 0;

    std::atomic<bool> m_IsPlaying = false;

    std::thread         m_DecodeThread;
    std::atomic<bool>   m_ThreadRunning = false;
    std::mutex          m_FrameMutex;
    std::deque<cv::Mat> m_FrameQueue;
    const size_t        MAX_QUEUE_SIZE = 10;

    sg_image    m_VideoTexture   = {SG_INVALID_ID};
    sg_view     m_VideoView      = {SG_INVALID_ID};
    ImTextureID m_VideoTextureID = 0;
    int         m_TexWidth       = 0;
    int         m_TexHeight      = 0;

    double m_VideoFPS        = 0.0;
    double m_TimeAccumulator = 0.0;
};
