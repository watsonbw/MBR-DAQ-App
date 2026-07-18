#pragma once

#include <memory>

#include <gsl/span>

#include "app/pages/page.hpp"
#include "app/pages/utils.hpp"

namespace mbr {

class ShockPage : public Page {
  public:
    explicit ShockPage(const std::shared_ptr<app_context>& ctx) : Page{ctx}, m_TextUtils{ctx} {}
    ~ShockPage() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

    void DrawLHS(const std::vector<std::string>& raw_lines);
    void DrawRHS(gsl::span<const double> time,
                 gsl::span<const double> fr,
                 gsl::span<const double> fl,
                 gsl::span<const double> br,
                 gsl::span<const double> bl);

  private:
    pages::utils::text_drawers m_TextUtils;
    std::string                m_DownloadFDText;
};

} // namespace mbr
