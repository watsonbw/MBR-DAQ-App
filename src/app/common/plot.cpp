#include <implot.h>

#include "app/common/plot.hpp"

template <typename T>
void PlotUtils::PlotIfNonEmpty(const char*           label,
                               const std::vector<T>& x,
                               const std::vector<T>& y,
                               bool                  extra_condition,
                               size_t                data_clip) {
    if (!x.empty() && !y.empty() && extra_condition) {
        const size_t n     = std::min(x.size(), y.size());
        const size_t count = (n > data_clip) ? (n - data_clip) : 0;
        ImPlot::PlotLine(label, x.data(), y.data(), count);
    }
}

template void PlotUtils::PlotIfNonEmpty<double>(
    const char*, const std::vector<double>&, const std::vector<double>&, bool, size_t);
