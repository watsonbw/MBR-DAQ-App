#include <vector>

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

App::App([[maybe_unused]] int arc, [[maybe_unused]] char* argv[])
    : m_Context{std::make_shared<AppContext>()} {
    Log::Init();
    ix::initNetSystem();

    const std::vector<std::string> packet_fields = {"T", "W", "E", "fr", "fl", "br", "bl"};
    m_GUI                                        = std::make_unique<GUI>(m_Context);
    m_Context->Backend = std::make_unique<TelemetryBackend>(packet_fields);
}

App::~App() { ix::uninitNetSystem(); }

void App::Run() {
    auto app_desc = m_GUI->GetSokolDesc();
    sapp_run(&app_desc);
}
