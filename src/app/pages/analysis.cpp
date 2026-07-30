#include "app/pages/analysis.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <gsl/util>

#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>
#include <tinyfiledialogs.h>

#include "app/assets/images/image_buttons.hpp"
#include "app/pages/page.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

using namespace std::chrono_literals;

namespace mbr::ui::pages {

void analysis_page::on_enter() { log_info(context_->log, "Entered AnalysisPage"); }
void analysis_page::on_exit() { log_info(context_->log, "Exited AnalysisPage"); }

void analysis_page::build_page() { PROFILE_FUNCTION(); }

} // namespace mbr::ui::pages
