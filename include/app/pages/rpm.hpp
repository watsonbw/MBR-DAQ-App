#pragma once

#include <memory>

#include <gsl/span>

#include "app/pages/page.hpp"
#include "app/pages/utils.hpp"

namespace mbr::pages {

class rpm_page : public page_base {
  public:
    explicit rpm_page(const std::shared_ptr<app_context>& ctx)
        : page_base{ctx}, text_drawer_{ctx} {}
    ~rpm_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void update() override;

  private:
    void DrawLHS(const std::vector<std::string>& raw_lines);
    void DrawRHS(gsl::span<const double> time,
                 gsl::span<const double> wheel,
                 gsl::span<const double> engine);

  private:
    pages::utils::text_drawers text_drawer_;
    std::string                download_fd_text_;
};

} // namespace mbr::pages
