#pragma once

#include <iostream>
#include <memory>

#include "app/pages/page.hpp"

struct AppContext;

class RPMPage : public Page {
  public:
    RPMPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~RPMPage() = default;

    inline virtual void OnEnter() override { std::cout << "[INFO]: Entered RPMPage" << "\n"; }
    virtual void        Update() override;
    inline virtual void OnExit() override { std::cout << "[INFO]: Exitted RPMPage" << "\n"; }
};
