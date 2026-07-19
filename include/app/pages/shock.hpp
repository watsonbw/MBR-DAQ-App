#pragma once

#include <memory>
#include <string>
#include <vector>

#include <gsl/span>

#include "app/pages/page.hpp"
#include "app/pages/utils.hpp"

namespace mbr::pages {

class shock_page : public page {
  public:
    explicit shock_page(const std::shared_ptr<app_context>& ctx) : page{ctx}, text_utils_{ctx} {}
    ~shock_page() override = default;

    void on_enter() override;
    void on_exit() override;
    void update() override;

    void draw_lhs(const std::vector<std::string>& raw_lines);
    void draw_rhs(gsl::span<const double> time,
                  gsl::span<const double> fr,
                  gsl::span<const double> fl,
                  gsl::span<const double> br,
                  gsl::span<const double> bl);

  private:
    pages::utils::text_drawers text_utils_;
    std::string                download_fd_text_;
};

} // namespace mbr::pages
