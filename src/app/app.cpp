#include "app/app.hpp"

#include <memory>

#include <ixwebsocket/IXNetSystem.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <stdx/types.hh>

#include "app/context.hpp"
#include "app/gui.hpp"
#include "core/log.hpp"
#include "esp32/backend.hpp"
#include "esp32/data.hpp"

namespace mbr {

app_t::app_t(i32, char**) : context_{std::make_shared<app_context>()} /*, m_Manager{m_Context}*/ {
    context_->log = context_->logger.get_log_fn();
    ix::initNetSystem();
    gui_              = std::make_unique<gui_t>(context_);
    context_->backend = std::make_unique<telemetry_backend>(context_->log);
}

app_t::~app_t() { ix::uninitNetSystem(); }

void app_t::run() {
    auto app_desc = gui_->get_sokol_desc();
    sapp_run(&app_desc);
}

} // namespace mbr
