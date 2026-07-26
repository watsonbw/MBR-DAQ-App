#include "app/app.hpp"

#include <filesystem>
#include <memory>

#include <ixwebsocket/IXNetSystem.h>
#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>
#include <stdx/types.hh>
#include <QApplication>

#include "app/context.hpp"
#include "app/gui.hpp"
#include "core/log.hpp"
#include "esp32/backend.hpp"
#include "esp32/data.hpp"

namespace mbr {

namespace { const std::filesystem::path DEFAULT_JSON_PATH{"MBR_data.json"}; } // namespace

app_t::app_t(i32 argc, char** argv) : qt_app_{std::make_unique<QApplication>(argc, argv)}, context_{std::make_shared<app_context>()}  /*, m_Manager{m_Context}*/ {
    context_->log = context_->logger.get_log_fn();
    ix::initNetSystem();
    gui_              = std::make_unique<gui_t>(context_);
    context_->backend = std::make_unique<telemetry_backend>(DEFAULT_JSON_PATH, context_->log);
}

app_t::~app_t() { ix::uninitNetSystem(); }

void app_t::run() {
    gui_->show();
    qt_app_->exec();
}

} // namespace mbr
