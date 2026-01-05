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

    void DrawLHS(std::vector<std::string> raw_data);
    void DrawRHS(std::vector<double> time,
                 std::vector<double> fr,
                 std::vector<double> fl,
                 std::vector<double> br,
                 std::vector<double> bl);
};
