#pragma once

#include <memory>
#include <string>

#include <QLabel>
#include <QWidget>

#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "core/ip.hpp"

namespace mbr::ui::pages {

class home_page : public page {
  public:
    explicit home_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr)
        : page{ctx, parent} {
        build_page();
    }
    ~home_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void build_page() override;

  private:
    QWidget* build_rhs();
    QWidget* build_lhs();
    void     connect_signals();

  private:
    ipv4_t                                                                       previous_ip_;
    ipv4_t                                                                       ip_buf_;
    std::string                                                                  username_buf_;
    std::string                                                                  password_buf_;
    std::string                                                                  sd_name_;
    std::string                                                                  set_name_;
    ankerl::unordered_dense::map<QString, QLabel*>                               labels_;
    ankerl::unordered_dense::map<QString, std::chrono::steady_clock::time_point> last_updated_;
};

} // namespace mbr::ui::pages
