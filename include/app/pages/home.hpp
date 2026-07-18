#pragma once

#include <memory>
#include <string>

#include "core/ip.hpp"

#include "app/pages/page.hpp"

namespace mbr {

class HomePage : public Page {
  public:
    explicit HomePage(const std::shared_ptr<app_context>& ctx) : Page{ctx} {}
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
    ipv4_t      m_PreviousIp;
    ipv4_t      m_IpBuf;
    std::string m_UsernameBuf;
    std::string m_PasswordBuf;
    std::string m_SDName;
    std::string m_SetName;
};

} // namespace mbr
