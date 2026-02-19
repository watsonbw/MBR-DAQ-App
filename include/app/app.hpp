#pragma once

#include "esp32/serial.hpp"
#include <memory>

struct AppContext;
class GUI;

class App {
  public:
    explicit App(int arc, char* argv[]);
    ~App();

    void Run();

  private:
    std::unique_ptr<GUI>        m_GUI;
    std::shared_ptr<AppContext> m_Context;
    Serial                      m_Manager;
};
