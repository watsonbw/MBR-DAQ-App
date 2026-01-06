#pragma once

#include <memory>

#include "app/pages/common/text.hpp"
#include "app/pages/page.hpp"

struct AppContext;

class RPMPage : public Page {
  public:
    RPMPage(std::shared_ptr<AppContext> ctx) : Page{ctx}, m_TextUtils{ctx} {}
    virtual ~RPMPage() = default;

    virtual void OnEnter() override;
    virtual void OnExit() override;
    virtual void Update() override;

  private:
    void DrawLHS(const std::vector<std::string>& raw_lines);
    void DrawRHS(const std::vector<double>& time,
                 const std::vector<double>& wheel,
                 const std::vector<double>& engine);

  private:
    TextUtils   m_TextUtils;
    std::string m_DownloadFDText;
};
