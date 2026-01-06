#pragma once

#include <vector>

class PlotUtils {
  public:
    template <typename T>
    static void PlotIfNonEmpty(const char*           label,
                               const std::vector<T>& x,
                               const std::vector<T>& y,
                               bool                  extra_condition = true,
                               size_t                data_clip       = 0);
};
