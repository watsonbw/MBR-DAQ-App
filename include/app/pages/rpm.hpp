#pragma once

#include <memory>

#include "app/pages/common/text.hpp"
#include "app/pages/page.hpp"

struct AppContext;

class RPMPage : public Page {
  public:
    explicit RPMPage(const std::shared_ptr<AppContext>& ctx) : Page{ctx}, m_TextUtils{ctx} {}
    ~RPMPage() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

  private:
    void DrawLHS(const std::vector<std::string>& raw_lines);
    void DrawRHS(const std::vector<double>& time,
                 const std::vector<double>& wheel,
                 const std::vector<double>& engine);

  private:
    TextUtils   m_TextUtils;
    std::string m_DownloadFDText;
};
