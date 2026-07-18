#pragma once

#include <gsl/span>
#include <implot.h>

namespace mbr::plot_utils {

template <typename T>
static void plot_if_non_empty(const char*        label,
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

} // namespace mbr::plot_utils
