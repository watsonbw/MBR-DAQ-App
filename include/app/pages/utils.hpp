#pragma once

#include <algorithm>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <gsl/span>
#include <imgui.h>
#include <implot.h>
#include <stdx/option.hh>

namespace mbr {

struct app_context;

namespace pages::utils {

template <typename T>
void plot_if_non_empty(const char*        label,
                       gsl::span<const T> x,
                       gsl::span<const T> y,
                       bool               extra_condition    = true,
                       size_t             data_clip_position = 0) {
    if (!x.empty() && !y.empty() && extra_condition) {
        const size_t n     = std::min(x.size(), y.size());
        const size_t count = n - std::min(n, data_clip_position);
        ImPlot::PlotLine(label, x.data(), y.data(), count);
    }
}

class text_drawers {
  public:
    explicit text_drawers(const std::shared_ptr<app_context>& ctx) : context_{ctx} {}

    void start_logging_button();
    bool start_serial_button();
    void send_data_button();

    // Appends the buf data to the filepath, clearing the buf on success.
    void data_download_button(const std::vector<std::string>& raw_lines, std::string& buf);

  private:
    std::shared_ptr<app_context> context_;
};

bool draw_input_box(const char*                label,
                    std::string&               buf,
                    stdx::option<const char*> hint        = stdx::none,
                    float                      width_scale = 200.0F,
                    ImGuiInputTextFlags        flags       = 0);
void draw_data_log(gsl::span<const std::string> raw_lines);

} // namespace pages::utils

} // namespace mbr
