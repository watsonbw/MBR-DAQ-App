#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>

#include "core/log.hpp"

#include "app/app.hpp"
#include "app/context.hpp"
#include "app/gui.hpp"

#include "esp32/backend.hpp"
#include "esp32/data.hpp"

App::App() : m_Context{std::make_shared<AppContext>()} {
    Log::Init();
    m_Backend = std::make_unique<TelemetryBackend>(m_Context);
    m_GUI     = std::make_unique<GUI>(m_Context);
}

App::~App() = default;

void App::Run() {
    m_Backend->Start();
    auto app_desc = m_GUI->GetSokolDesc();
    sapp_run(&app_desc);
}
