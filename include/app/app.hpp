#pragma once

#include <memory>

struct AppContext;
class GUI;


class App {
  public:
    App();
    ~App();

    void Run();

  private:
    std::unique_ptr<GUI>              m_GUI;
    std::shared_ptr<AppContext>       m_Context;
};
