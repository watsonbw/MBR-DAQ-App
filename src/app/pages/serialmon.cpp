#include "app/pages/serialmon.hpp"

#include <string>

#include <gsl/util>
#include <imgui.h>
#include <stdx/enum.hh>
#include <stdx/profiler.hh>
#include <stdx/types.hh>

#include "app/pages/utils.hpp"
#include "app/style.hpp"
#include "core/log.hpp"
#include "esp32/serial.hpp"

namespace mbr::pages {

void serial_page::on_enter() { log_info(context_->log, "Entered SerialPage"); }
void serial_page::on_exit() { log_info(context_->log, "Exited SerialPage"); }

void serial_page::update() {
    PROFILE_FUNCTION();
}

} // namespace mbr::pages
