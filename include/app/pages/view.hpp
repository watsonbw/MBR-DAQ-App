#pragma once

#include <iostream>
#include <memory>

#include "app/pages/page.hpp"

class ViewPage : public Page {
  public:
    ViewPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ViewPage() = default;

    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void OnExit() override;
};
