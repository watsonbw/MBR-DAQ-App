#pragma once

#include <iostream>
#include <memory>

#include "app/pages/page.hpp"

class HomePage : public Page {
  public:
    HomePage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~HomePage() = default;

    inline virtual void OnEnter() override { std::cout << "[INFO]: Entered HomePage" << "\n"; }
    virtual void        Update() override;
    inline virtual void OnExit() override { std::cout << "[INFO]: Exitted HomePage" << "\n"; }
};
