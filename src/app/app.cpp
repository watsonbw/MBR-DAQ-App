#include "app/app.hpp"
#include "app/context.hpp"
#include "app/gui.hpp"

#include "esp32/data.hpp"

App::App() : m_Context{std::make_shared<AppContext>()} {}

void App::Run() {
    TelemetryData backend{m_Context};
    backend.Start();

    GUI gui{m_Context};
    gui.Launch();
}
