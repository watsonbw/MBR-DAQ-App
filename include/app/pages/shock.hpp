#pragma once

#include <memory>

#include "app/pages/common/text.hpp"
#include "app/pages/page.hpp"

class ShockPage : public Page {
  public:
    explicit ShockPage(const std::shared_ptr<AppContext>& ctx) : Page{ctx}, m_TextUtils{ctx} {}
    ~ShockPage() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

    void DrawLHS(const std::vector<std::string>& raw_lines);
    void DrawRHS(const std::vector<double>& time,
                 const std::vector<double>& fr,
                 const std::vector<double>& fl,
                 const std::vector<double>& br,
                 const std::vector<double>& bl);

  private:
    TextUtils   m_TextUtils;
    std::string m_DownloadFDText;
};
