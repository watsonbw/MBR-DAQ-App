#pragma once

#include <memory>

#include "app/pages/page.hpp"

class HomePage : public Page {
  public:
    HomePage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~HomePage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;
};
