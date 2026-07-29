#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <QWidget>
#include <stdx/option.hh>
#include <stdx/types.hh>

#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "core/time.hpp"

namespace mbr::pages {

class analysis_page : public page {
  public:
    using selected_video_t    = stdx::option<std::pair<std::string, stdx::option<date_time>>>;
    using selected_txt_file_t = stdx::option<std::string>;

    enum class data_view_t : u8 {
        ALL,
        RPMDATA,
        SHOCKDATA,
    };

    [[nodiscard]] static const char* data_type_string(data_view_t type);

  public:
    explicit analysis_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr)
        : page(ctx, parent) {};
    ~analysis_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void build_page() override;

  private:
  private:
};

} // namespace mbr::pages
