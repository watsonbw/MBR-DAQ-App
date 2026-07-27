#pragma once

#include <memory>
#include <string>

#include <QWidget>

#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "core/ip.hpp"

namespace mbr::pages {

class home_page : public page {
  public:
    explicit home_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr) : page{ctx, parent} {}
    ~home_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void update() override;

  private:


  private:
    ipv4_t      previous_ip_;
    ipv4_t      ip_buf_;
    std::string username_buf_;
    std::string password_buf_;
    std::string sd_name_;
    std::string set_name_;
};

} // namespace mbr::pages
