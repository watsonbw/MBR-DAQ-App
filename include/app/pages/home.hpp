#pragma once

#include <memory>
#include <string>

#include "core/ip.hpp"

#include "app/pages/page.hpp"

class HomePage : public Page {
  public:
    explicit HomePage(const std::shared_ptr<AppContext>& ctx)
        : Page{ctx}, m_PreviousIp{DEFAULT_IP} {}
    ~HomePage() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

  private:
    void DrawTopLHS();
    void DrawTopRHS();

    void DrawBottomLHS();
    void DrawBottomRHS();
    void DrawIPControls();
    void DrawCredentialControls();

  private:
    IpV4        m_PreviousIp;
    IpV4        m_IpBuf;
    std::string m_UsernameBuf;
    std::string m_PasswordBuf;
};
