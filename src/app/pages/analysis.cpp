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
#include <imgui.h>
#include <imgui_internal.h>
#include <implot.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <stdx/assert.hh>
#include <stdx/option.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>
#include <tinyfiledialogs.h>

#include "app/assets/images/image_buttons.hpp"
#include "app/pages/page.hpp"
#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"
#include "core/time.hpp"

using namespace std::chrono_literals;

namespace mbr::pages {

    void analysis_page::on_enter() { log_info(context_->log, "Entered AnalysisPage"); }
    void analysis_page::on_exit() { log_info(context_->log, "Exited AnalysisPage"); }

    void analysis_page::update() {
        PROFILE_FUNCTION();
    }
} // namespace mbr::pages
