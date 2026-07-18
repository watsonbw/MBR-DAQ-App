#pragma once

#include <memory>
#include <string>

#include "core/ip.hpp"

#include "app/pages/page.hpp"

namespace mbr::pages {

class home_page : public page_base {
  public:
    explicit home_page(const std::shared_ptr<app_context>& ctx) : page_base{ctx} {}
    ~home_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void update() override;

  private:
    void draw_top_lhs();
    void draw_top_rhs();

    void draw_bottom_lhs();
    void draw_bottom_rhs();
    void draw_ip_controls();
    void draw_credential_controls();

  private:
    ipv4_t      previous_ip_;
    ipv4_t      ip_buf_;
    std::string username_buf_;
    std::string password_buf_;
    std::string sd_name_;
    std::string set_name_;
};

} // namespace mbr::pages
