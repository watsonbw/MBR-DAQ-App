#pragma once

#include <memory>
#include <optional>
#include <string>

// #include <opencv2/opencv.hpp>

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
    // std::string      video_path;
    // cv::VideoCapture cap;
    // cv::Mat          frame;
    // sg_image         video_texture = {0};
};

// void FtoT(const cv::Mat& frame, sg_image tex);
