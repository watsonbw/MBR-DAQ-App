#pragma once

#include <memory>

#include "app/context.hpp"

namespace mbr::pages {

class page {
  public:
    page()          = default;
    virtual ~page() = default;

    virtual void on_enter() = 0;
    virtual void on_exit()  = 0;
    virtual void update()   = 0;

  protected:
    explicit page(const std::shared_ptr<app_context>& ctx) : context_{ctx} {}
    std::shared_ptr<app_context> context_;
};

} // namespace mbr::pages
