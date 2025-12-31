#pragma once

#include <memory>

#include "app/pages/page.hpp"

class ShockPage : public Page {
  public:
    ShockPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ShockPage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;
};
