#pragma once

#include <memory>

struct AppContext;

enum PageType {
    HOME,
    RPM,
    SHOCK,
    VIEW,
};

const char* PageTypeString(PageType type);

class Page {
  public:
    Page()          = default;
    virtual ~Page() = default;

    virtual void OnEnter() = 0;
    virtual void OnExit()  = 0;
    virtual void Update()  = 0;

  protected:
    explicit Page(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {}
    std::shared_ptr<AppContext> m_Context;
};

int DefaultWindowFlags();
