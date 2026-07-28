#pragma once

#include <memory>
#include <string>

#include <stdx/types.hh>
#include <QWidget>

#include "app/pages/page.hpp"

namespace mbr::pages {

class serial_page : public page {
  public:
    explicit serial_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr) : page{ctx, parent} {}
    ~serial_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void build_page() override;

  private:

  private:
    std::string         serial_buffer_;
};

} // namespace mbr::pages
