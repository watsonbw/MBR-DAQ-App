#pragma once

#include <memory>

struct AppContext;
class GUI;
class TelemetryBackend;

class App {
  public:
    App();
    ~App();

    void Run();

  private:
    std::unique_ptr<GUI>              m_GUI;
    std::unique_ptr<TelemetryBackend> m_Backend;
    std::shared_ptr<AppContext>       m_Context;
};
