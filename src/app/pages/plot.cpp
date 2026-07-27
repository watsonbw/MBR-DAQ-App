#include "app/pages/plot.hpp"

#include <shared_mutex>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <gsl/span>
#include <gsl/util>
#include <imgui.h>
#include <implot.h>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"

namespace mbr::pages {

void plot_page::on_enter() { log_info(context_->log, "Entered PlotPage"); }
void plot_page::on_exit() { log_info(context_->log, "Exited PlotPage"); }

void plot_page::update() {
}

} // namespace mbr::pages
