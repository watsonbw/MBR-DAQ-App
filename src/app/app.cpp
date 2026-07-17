#include <sokol_app.h>
#include <sokol_gfx.h>
#include <sokol_glue.h>
#include <sokol_imgui.h>

#include <ixwebsocket/IXNetSystem.h>

#include "core/log.hpp"

#include "app/app.hpp"
#include "app/context.hpp"
#include "app/gui.hpp"

#include "esp32/backend.hpp"
#include "esp32/data.hpp"

namespace mbr {

App::App([[maybe_unused]] int arc, [[maybe_unused]] char* argv[])
    : m_Context{std::make_shared<AppContext>()} /*, m_Manager{m_Context}*/ {
    Log::Init();
    ix::initNetSystem();
    m_GUI              = std::make_unique<GUI>(m_Context);
    m_Context->Backend = std::make_unique<TelemetryBackend>();
}

App::~App() { ix::uninitNetSystem(); }

void App::Run() {
    auto app_desc = m_GUI->GetSokolDesc();
    sapp_run(&app_desc);
}

} // namespace mbr
