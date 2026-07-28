#pragma once

#include <memory>
#include <string>

#include <QWidget>

#include "app/context.hpp"
#include "app/pages/page.hpp"
#include "core/ip.hpp"

namespace mbr::pages {

class settings_page : public page {
  public:
    explicit settings_page(const std::shared_ptr<app_context>& ctx, QWidget* parent = nullptr) : page{ctx, parent} {}
    ~settings_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void build_page() override;

  private:


  private:

};

} // namespace mbr::pages
