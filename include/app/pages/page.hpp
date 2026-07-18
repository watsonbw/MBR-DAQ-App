#pragma once

#include <memory>

#include "app/context.hpp"

namespace mbr {

class Page {
  public:
    Page()          = default;
    virtual ~Page() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit()  = 0;
    virtual void Update()  = 0;

  protected:
    explicit Page(const std::shared_ptr<app_context>& ctx) : context_{ctx} {}
    std::shared_ptr<app_context> context_; // NOLINT
};

} // namespace mbr
