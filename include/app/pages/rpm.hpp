#pragma once

#include <memory>

#include "app/pages/page.hpp"

struct AppContext;

class RPMPage : public Page {
  public:
    RPMPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~RPMPage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;
    void         DrawLHS(std::vector<std::string> raw_data);
    void DrawRHS(std::vector<double> time, std::vector<double> wheel, std::vector<double> engine);
};
