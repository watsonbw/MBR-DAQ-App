#pragma once

#include <memory>

struct AppContext;

enum PageType {
    HOME,
    RPM,
    SHOCK,
    VIEW,
};

class Page {
  public:
    Page() = default;
    virtual ~Page() = default;

    virtual void OnEnter() = 0;
    virtual void Update()  = 0;
    virtual void OnExit()  = 0;

  protected:
    explicit Page(std::shared_ptr<AppContext> ctx) : m_Context{ctx} {}
    std::shared_ptr<AppContext> m_Context;
};

class HomePage : public Page {
  public:
    HomePage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~HomePage() = default;

    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void OnExit() override;
};

class RPMPage : public Page {
  public:
    RPMPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~RPMPage() = default;

    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void OnExit() override;
};

class ShockPage : public Page {
  public:
    ShockPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ShockPage() = default;

    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void OnExit() override;
};

class ViewPage : public Page {
  public:
    ViewPage(std::shared_ptr<AppContext> ctx) : Page{ctx} {}
    virtual ~ViewPage() = default;

    virtual void OnEnter() override;
    virtual void Update() override;
    virtual void OnExit() override;
};
