#pragma once

#include <memory>

#include <gsl/span>

#include "app/common/text.hpp"
#include "app/pages/page.hpp"

namespace mbr {

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
    void DrawRHS(gsl::span<const double> time,
                 gsl::span<const double> wheel,
                 gsl::span<const double> engine);

  private:
    TextUtils   m_TextUtils;
    std::string m_DownloadFDText;
};

} // namespace mbr
