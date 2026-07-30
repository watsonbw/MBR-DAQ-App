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

namespace mbr::ui::pages {

class analysis_page : public page {
  public:
    explicit analysis_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr)
        : page(ctx, parent) {};
    ~analysis_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void build_page() override;

  private:
};

} // namespace mbr::ui::pages
