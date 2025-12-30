#pragma once

#include <memory>

struct AppContext;

class App {
  public:
    App();
    ~App() = default;

    void Run();

  private:
    std::shared_ptr<AppContext> m_Context;
};
