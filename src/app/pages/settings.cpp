#include "app/pages/settings.hpp"

#include <shared_mutex>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <gsl/span>
#include <gsl/util>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "core/log.hpp"

namespace mbr::pages {

void settings_page::on_enter() { log_info(context_->log, "Entered SettingsPage"); }
void settings_page::on_exit() { log_info(context_->log, "Exited SettingsPage"); }

void settings_page::build_page() {
}

} // namespace mbr::pages
