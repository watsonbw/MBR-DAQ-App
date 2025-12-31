#pragma once

#include <memory>
#include <optional>
#include <string>

#include <opencv2/opencv.hpp>

#include "app/pages/page.hpp"

#include <GL/gl.h>

class ViewPage : public Page {
  public:
    ViewPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ViewPage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;

    std::optional<std::string> OpenFile();

  private:
    std::string      video_path;
    cv::VideoCapture cap;
    cv::Mat frame;
    GLuint video_texture = 0;
};
