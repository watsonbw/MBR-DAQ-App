#pragma once

#include <memory>
#include <optional>
#include <string>

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

    std::optional<std::string> OpenFile();

  private:
    void UpdateFrameToTexture();
    void TryCleanupSokolResources();

  private:
    std::string      m_VideoPath;
    cv::VideoCapture m_Cap;
    cv::Mat          m_RawFrame;
    cv::Mat          m_DisplayFrame;

    sg_image    m_VideoTexture   = {SG_INVALID_ID};
    sg_view     m_VideoView           = {SG_INVALID_ID};
    ImTextureID m_VideoTextureID = 0;

    double m_VideoFPS        = 0.0;
    double m_TimeAccumulator = 0.0;
};
