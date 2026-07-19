#pragma once

#include <memory>
#include <string>

#include <stdx/types.hh>

#include "app/pages/page.hpp"
#include "app/pages/utils.hpp"

namespace mbr::pages {

enum class baud_rate : u32 {
    THREE           = 300,
    TWELVE          = 1'200,
    TWENTYFOUR      = 2'400,
    FORTYEIGHT      = 4'800,
    NINETYSIX       = 9'600,
    ONENIGHTYTWO    = 19'200,
    THREEEIGHTYFOUR = 38'400,
    FIVESEVENTYSIX  = 57'600,
    ONEONEFIFTYTWO  = 115'200,
};

class serial_page : public page {
  public:
    explicit serial_page(const std::shared_ptr<app_context>& ctx) : page{ctx}, text_drawer_{ctx} {}
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
