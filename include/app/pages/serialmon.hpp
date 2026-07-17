#pragma once

#include <memory>
#include <string>

#include "app/common/text.hpp"
#include "app/pages/page.hpp"

namespace mbr {

enum class BaudRate : uint32_t {
    THREE           = 300,
    TWELVE          = 1'200,
    TWENTYFOUR      = 2'400,
    FORTYEIGHT      = 4'800,
    NINETYSIX       = 9'600,
    ONENIGHTYTWO    = 19'200,
    THREEEIGHTYFOUR = 38'400,
    FIVESEVENTYSIX  = 57'600,
    ONEONEFIFTYTWO  = 115'200,
};

class SerialPage : public Page {
  public:
    explicit SerialPage(const std::shared_ptr<AppContext>& ctx) : Page{ctx}, m_TextUtils{ctx} {}
    ~SerialPage() override = default;

    void OnEnter() override;
    void OnExit() override;
    void Update() override;

  private:
    void DrawTopLHS();
    void DrawTopRHS();

    void DrawBottom();

  private:
    TextUtils   m_TextUtils;
    std::string m_SerialBuffer;
};

} // namespace mbr
