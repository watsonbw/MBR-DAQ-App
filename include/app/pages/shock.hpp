#pragma once

#include <iostream>
#include <memory>

#include "app/pages/page.hpp"

class ShockPage : public Page {
  public:
    ShockPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ShockPage() = default;

    inline virtual void OnEnter() override { std::cout << "[INFO]: Entered ShockPage" << "\n"; }
    virtual void        Update() override;
    inline virtual void OnExit() override { std::cout << "[INFO]: Exitted ShockPage" << "\n"; }
};
