#pragma once

#include <memory>
#include <string>

#include <stdx/types.hh>
#include <QWidget>

#include "app/pages/page.hpp"
#include "app/pages/utils.hpp"

namespace mbr::pages {

class serial_page : public page {
  public:
    explicit serial_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr) : page{ctx, parent}, text_drawer_{ctx} {}
    ~serial_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void update() override;

  private:
    void draw_top_lhs();
    void draw_top_rhs();
    void draw_bottom();

  private:
    utils::text_drawers text_drawer_;
    std::string         serial_buffer_;
};

} // namespace mbr::pages
